#include <assembler/assembler.hpp>
#include <common/exceptions.hpp>

#include <iostream>
#include <iomanip>

constexpr uint32_t INVALID = 0;
constexpr uint32_t UNUSED = UINT32_MAX;
constexpr int UNUSED_INT = -1;

namespace
{
  constexpr int WORD_SIZE = 4;
}

namespace asm_core
{

Assembler::Assembler(const std::string& outputFilePath)
  : outputFilePath(outputFilePath)
{
  Symbol undefinedSection{"UND", 0, UNUSED_INT, false, false, false, 0};
  symbolTable.emplace_back(undefinedSection);
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertGlobalSymbol(const std::string& symbolName)
{
  const uint32_t symbolIndex = findSymbol(symbolName);
  
  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolTable.emplace_back(symbolName, INVALID, 0, true, false, false, UNUSED);
  }
  else // jeste u tabeli simbola
  {
    Symbol& symbol = symbolTable[symbolIndex];
    
    if(symbol.isExtern)
    {
      throw AssemblerError(ErrorCode::GLOBAL_EXTERN_CONFLICT);
    }

    symbol.isGlobal = true;
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertExternSymbol(const std::string& symbolName)
{
  const uint32_t symbolIndex = findSymbol(symbolName);

  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolTable.emplace_back(symbolName, INVALID, INVALID, false, true, false, UNUSED);
  }
  else // jeste u tabeli simbola
  {
    Symbol& symbol = symbolTable[symbolIndex];

    if(symbol.isDefined)
    {
      throw AssemblerError(ErrorCode::SYMBOL_REDECLARATION);
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::defineSymbol(const std::string& symbolName)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  uint32_t symbolIndex = findSymbol(symbolName);

  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolTable.emplace_back(symbolName, currentSectionNumber, locationCounter, false, false, true, UNUSED);
  }
  else // jeste u tabeli simbola
  {
    Symbol& symbol = symbolTable[symbolIndex];
    if(symbol.isDefined || symbol.isExtern)
    {
      throw AssemblerError(ErrorCode::SYMBOL_REDECLARATION);
    }
    
    symbol.sectionNumber = currentSectionNumber;
    symbol.isDefined = true;
    symbol.value = locationCounter;
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::openNewSection(const std::string& sectionName)
{
  const uint32_t symbolIndex = findSymbol(sectionName);

  if (symbolIndex != INVALID) // jeste u tabeli simbola, GRESKA
  {
    Symbol& symbol = symbolTable[symbolIndex];

    if(symbol.sectionNumber == symbolIndex) // redeklaracija sekcije
    {
      throw AssemblerError(ErrorCode::SECTION_REDECLARATION);
    }
    else // konflikt simbola i sekcije
    {
      throw AssemblerError(ErrorCode::SYMBOL_REDECLARATION);
    }
  }
  
  closeCurrentSection();

  currentSectionNumber = symbolTable.size();
  symbolTable.emplace_back(sectionName, currentSectionNumber, INVALID, false, false, false, 0);

}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertSymbol(const std::string& symbolName)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  uint32_t symbolIndex = findSymbol(symbolName);
  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolIndex = symbolTable.size();
    symbolTable.emplace_back(symbolName, INVALID, INVALID, false, false, false, UNUSED);
  }

  // ubacujemo u niz koriscenja
  Symbol& symbol = symbolTable[symbolIndex];
  symbol.symbolUsages.emplace_back(SymbolUsageType::IMM, currentSectionNumber, locationCounter);

  locationCounter += WORD;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertLiteral(uint32_t value)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  sectionMemory.writeWord(value);

  locationCounter += WORD;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::endAssembly()
{
  closeCurrentSection();
  // backpatching
  // pravljenje tabele relokacija
  printTables();
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::printTables()
{
  // ispis tabele simbola
  std::string title = "SYMBOL TABLE";
  int tableWidth = 116;
  int titlePadding = (tableWidth - title.length()) / 2;


  std::cout << std::string(tableWidth, '-') << "\n";
  std::cout << std::string(titlePadding, ' ') << title << std::string(tableWidth - titlePadding - title.length(), ' ') << "\n";
  std::cout << std::string(tableWidth, '-') << "\n";

  std::cout << std::setw(6) << "index" << "|"
            << std::setw(15) << "name" << "|"
            << std::setw(15) << "sectionNumber" << "|"
            << std::setw(15) << "value" << "|"
            << std::setw(10) << "isGlobal" << "|"
            << std::setw(10) << "isExtern" << "|"
            << std::setw(10) << "isDefined" << "|"
            << std::setw(15) << "size" << "|"
            << std::setw(10) << "numUsages" << "|\n";

  for(uint32_t i = 0, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    std::cout << std::setw(6) << i << "|"
              << std::setw(15) << symbolTable[i].name << "|"
              << std::setw(15) << symbolTable[i].sectionNumber << "|"
              << std::setw(15) << symbolTable[i].value << "|"
              << std::setw(10) << (symbolTable[i].isGlobal ? "true" : "false") << "|"
              << std::setw(10) << (symbolTable[i].isExtern ? "true" : "false") << "|"
              << std::setw(10) << (symbolTable[i].isDefined ? "true" : "false") << "|"
              << std::setw(15) << symbolTable[i].size << "|"
              << std::setw(10) << symbolTable[i].symbolUsages.size() << "|\n"
              ;
  }
  std::cout << std::string(tableWidth, '-') << "\n";
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Assembler::findSymbol(const std::string& symbolName) const
{
  for(uint32_t i = 0, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    if(symbolTable[i].name == symbolName)
    {
      return i;
    }
  }

  return INVALID;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::closeCurrentSection()
{
  if(currentSectionNumber == INVALID)
  {
    return;
  }
  
  // obrada stare sekcije
  const SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  symbolTable[currentSectionNumber].size = sectionMemory.getSectionSize();
  
  // resetovanje podataka
  locationCounter = 0;
  currentSectionNumber = INVALID;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::backpatch()
{

}
//-----------------------------------------------------------------------------------------------------------
void Assembler::createRelocationTables()
{
  
}


} // namespace asm_core