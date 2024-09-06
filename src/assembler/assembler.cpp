#include <assembler/assembler.hpp>
#include <common/exceptions.hpp>

#include <iostream>
#include <iomanip>

constexpr uint32_t INVALID = 0;
constexpr uint32_t UNUSED = UINT32_MAX;
constexpr int UNUSED_INT = -1;

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
    symbolTable.emplace_back(symbolName, INVALID, INVALID, true, false, false, UNUSED);
  }
  else // jeste u tabeli simbola
  {
    Symbol& symbol = symbolTable[symbolIndex];
    
    if(symbol.isExtern)
    {
      throw common::AssemblerError(common::ErrorCode::GLOBAL_EXTERN_CONFLICT);
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
      throw common::AssemblerError(common::ErrorCode::SYMBOL_REDECLARATION);
    }
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
      throw common::AssemblerError(common::ErrorCode::SECTION_REDECLARATION);
    }
    else // konflikt simbola i sekcije
    {
      throw common::AssemblerError(common::ErrorCode::SYMBOL_REDECLARATION);
    }
  }
  
  closeCurrentSection();

  currentSectionNumber = symbolTable.size();
  symbolTable.emplace_back(sectionName, currentSectionNumber, INVALID, false, false, false, 0);
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::endAssembly()
{
  closeCurrentSection();
  
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
  // TODO: bazen literala, obrada sekcije koju zatvaramo...

  symbolTable[currentSectionNumber].size = locationCounter;
  
  locationCounter = 0;
  currentSectionNumber = INVALID;
}

} // namespace asm_core