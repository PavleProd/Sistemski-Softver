#include <assembler/assembler_tables_printer.hpp>

#include <common/exceptions.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

namespace
{

std::string replaceExtension(const std::string& fileName, const std::string& newExtension)
{
  size_t dotPosition = fileName.find_last_of('.');
  if(dotPosition != std::string::npos)
  {
    return fileName.substr(0, dotPosition) + newExtension;
  }

  return fileName;
}

void printMemorySegment(const SectionMemory::MemorySegment& segment, const std::string& segmentName, uint32_t& address)
{
  std::cout << segmentName << ":\n";
  
  for (size_t i = 0; i < segment.size(); i += 4) {
      std::cout << std::setw(4) << std::setfill('0') << std::hex << address << ": ";
      for (size_t j = 0; j < 4 && (i + j) < segment.size(); ++j) 
      {
          std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(segment[i + j]) << " ";
      }
      std::cout << "\n";

      address += 4;
  }
  std::cout << std::dec;
}


} // unammed

namespace asm_core
{

void AssemblerTablesPrinter::printTables(const common::AssemblerOutputData& data, const std::string& filePath)
{
  std::string objDumpPath = replaceExtension(filePath, ".objdump");
  std::ofstream outFile(objDumpPath);
  if (!outFile.is_open()) 
  {
    throw RuntimeError("Couldn't open output file " + objDumpPath + " in writing mode!");
  }
}

void AssemblerTablesPrinter::printSymbolTable(const std::vector<common::Symbol>& symbolTable, std::ofstream& outFile)
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

void AssemblerTablesPrinter::printRelocationTables(const common::AssemblerOutputData& data, std::ofstream& outFile)
{
  std::cout << "\n==RELOCATION TABLES==\n\n";
  for (const auto& [sectionNumber, relocations] : data.sectionRelocationMap)
  {
    std::cout << "Section: " << data.symbolTable[sectionNumber].name << "\n";

    std::cout << std::setw(6) << "Index" << " | "
              << std::setw(15) << "Instruction OC" << " | "
              << std::setw(10) << "Offset" << " | "
              << std::setw(20) << "SymbolTableReference" << " |\n";
    std::cout << "----------------------------------------------------------------\n";

    for (size_t i = 0; i < relocations.size(); ++i) {
        const auto& entry = relocations[i];

        std::cout << std::setw(6) << i << " | "
                  << std::setw(15) << static_cast<int>(entry.instruction.oc) << " | "
                  << std::setw(10) << entry.offset << " | "
                  << std::setw(20) << entry.symbolTableReference << " |\n";
    }

    std::cout << "----------------------------------------------------------------\n";
  }
}

void AssemblerTablesPrinter::printGeneratedCode(const common::AssemblerOutputData& data, std::ofstream& outFile)
{
  std::cout << "\n==GENERATED CODE BY SECTION==\n\n";
  for (const auto& [sectionNumber, sectionMemory] : data.sectionMemoryMap) 
  {
    std::cout << "Section: " << data.symbolTable[sectionNumber].name << "\n";
    
    uint32_t address = 0;

    const auto& code = sectionMemory.getCode();
    printMemorySegment(code, "Code", address);

    const auto& literalPool = sectionMemory.getLiteralPool();
    printMemorySegment(literalPool, "Literal Pool", address);

    std::cout << "-----------------------\n";
  }
}

} // asm_core
