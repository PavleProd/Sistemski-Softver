#include <assembler/assembler.hpp>

#include <common/assembler_common.hpp>
#include <common/exceptions.hpp>

#include <iostream>
#include <iomanip>

constexpr uint32_t INVALID = 0;
constexpr uint32_t UNUSED = UINT32_MAX;
constexpr int UNUSED_INT = -1;

namespace
{

constexpr int WORD_SIZE = 4;

void printMemorySegment(const std::vector<uint8_t>& segment, const std::string& segmentName, uint32_t& address)
{
    std::cout << segmentName << ":\n";
    
    for (size_t i = 0; i < segment.size(); i += 4) {
        std::cout << std::setw(8) << std::setfill('0') << std::hex << address << ": ";
        for (size_t j = 0; j < 4 && (i + j) < segment.size(); ++j) 
        {
            std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(segment[i + j]) << " ";
        }
        std::cout << "\n";

        address += 4;
    }
    std::cout << std::dec;
}

} // unnamed

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
  symbol.symbolUsages.emplace_back(SymbolUsageType::SYM_IMM, currentSectionNumber, locationCounter);

  locationCounter += WORD_SIZE;
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

  locationCounter += WORD_SIZE;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertBSS(uint32_t numBytes)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  sectionMemory.writeBSS(numBytes);

  locationCounter += numBytes;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::endAssembly()
{
  closeCurrentSection();
  backpatch();
  createRelocationTables();
  printTables();
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::printTables() const
{
  printSymbolTable();
  printRelocationTables();
  printGeneratedCode();
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
/*
Prolazi kroz sva koriscenja simbola u tabeli simbola i ako je:
  - lokalni simbol: upisuje njegovu vrednost
  - globalni simbol: upisuje 0 
*/ 
void Assembler::backpatch()
{
  for(int i = 1, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    const Symbol& symbol = symbolTable[i]; 
    if(symbol.sectionNumber == i) // simbol je sekcija
    {
      continue;
    }

    // TODO: odradi za sve tipove
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::createRelocationTables()
{
  for(int i = 1, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    const Symbol& symbol = symbolTable[i]; 
    if(symbol.sectionNumber == i) // simbol je sekcija
    {
      continue;
    }

    for(const SymbolUsage& usage : symbol.symbolUsages)
    {
      // za sekciju gde se koristi simbol pravimo referencu ka mestu gde je simbol definisan
      // TODO: odradi za sve tipove, koja je razlika izmedju tipova uopste
      uint32_t symbolTableReference = (symbol.isGlobal || symbol.isExtern) ? i : symbol.sectionNumber;
      sectionRelocationMap[usage.sectionNumber]
        .emplace_back(usage.symbolUsageType, usage.sectionOffset, symbolTableReference);
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::printSymbolTable() const
{
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
void Assembler::printRelocationTables() const
{
  std::cout << "\n==RELOCATION TABLES==\n\n";
  for (const auto& [sectionNumber, relocations] : sectionRelocationMap)
  {
        std::cout << "Section: " << symbolTable[sectionNumber].name << "\n";

        std::cout << std::setw(6) << "Index" << " | "
                  << std::setw(15) << "SymbolUsageType" << " | "
                  << std::setw(10) << "Offset" << " | "
                  << std::setw(20) << "SymbolTableReference" << " |\n";
        std::cout << "----------------------------------------------------------------\n";

        for (size_t i = 0; i < relocations.size(); ++i) {
            const auto& entry = relocations[i];

            std::cout << std::setw(6) << i << " | "
                      << std::setw(15) << static_cast<int>(entry.symbolUsageType) << " | "
                      << std::setw(10) << entry.offset << " | "
                      << std::setw(20) << entry.symbolTableReference << " |\n";
        }

        std::cout << "----------------------------------------------------------------\n";
    }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::printGeneratedCode() const
{
  std::cout << "\n==GENERATED CODE BY SECTION==\n\n";
  for (const auto& [sectionNumber, sectionMemory] : sectionMemoryMap) 
  {
        std::cout << "Section: " << symbolTable[sectionNumber].name << "\n";
        
        uint32_t address = 0;

        const auto& code = sectionMemory.getCode();
        printMemorySegment(code, "Code", address);

        const auto& literalPool = sectionMemory.getLiteralPool();
        printMemorySegment(literalPool, "Literal Pool", address);

        std::cout << "-----------------------\n";
    }
}


} // namespace asm_core