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

void printMemorySegment(
  const SectionMemory::MemorySegment& segment,
  const std::string& segmentName,
  uint32_t& address,
  std::ofstream& outFile)
{
  outFile << segmentName << ":\n";
  
  for (size_t i = 0; i < segment.size(); i += 4) {
      outFile << std::setw(4) << std::setfill('0') << std::hex << address << ": ";
      for (size_t j = 0; j < 4 && (i + j) < segment.size(); ++j) 
      {
          outFile << std::setw(2) << std::setfill('0') << static_cast<int>(segment[i + j]) << " ";
      }
      outFile << "\n";

      address += 4;
  }
  outFile << std::dec;
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

  printSymbolTable(data.symbolTable, outFile);
  printRelocationTables(data, outFile);
  printGeneratedCode(data, outFile);
}

void AssemblerTablesPrinter::printSymbolTable(const std::vector<common::Symbol>& symbolTable, std::ofstream& outFile)
{
  std::string title = "SYMBOL TABLE";
  int tableWidth = 116;
  int titlePadding = (tableWidth - title.length()) / 2;


  outFile << std::string(tableWidth, '-') << "\n";
  outFile << std::string(titlePadding, ' ') << title << std::string(tableWidth - titlePadding - title.length(), ' ') << "\n";
  outFile << std::string(tableWidth, '-') << "\n";

  outFile << std::setw(6) << "index" << "|"
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
    outFile << std::setw(6) << i << "|"
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
  outFile << std::string(tableWidth, '-') << "\n";
}

void AssemblerTablesPrinter::printRelocationTables(const common::AssemblerOutputData& data, std::ofstream& outFile)
{
  outFile << "\n==RELOCATION TABLES==\n\n";
  for (const auto& [sectionNumber, relocations] : data.sectionRelocationMap)
  {
    outFile << "Section: " << data.symbolTable[sectionNumber].name << "\n";

    outFile << std::setw(6) << "Index" << " | "
              << std::setw(15) << "Instruction OC" << " | "
              << std::setw(10) << "Offset" << " | "
              << std::setw(20) << "SymbolTableReference" << " |\n";
    outFile << "----------------------------------------------------------------\n";

    for (size_t i = 0; i < relocations.size(); ++i) {
        const auto& entry = relocations[i];

        outFile << std::setw(6) << i << " | "
                  << std::setw(15) << static_cast<int>(entry.operationCode) << " | "
                  << std::setw(10) << entry.offset << " | "
                  << std::setw(20) << entry.symbolTableReference << " |\n";
    }

    outFile << "----------------------------------------------------------------\n";
  }
}

void AssemblerTablesPrinter::printGeneratedCode(const common::AssemblerOutputData& data, std::ofstream& outFile)
{
  outFile << "\n==GENERATED CODE BY SECTION==\n\n";
  for (const auto& [sectionNumber, sectionMemory] : data.sectionMemoryMap) 
  {
    outFile << "Section: " << data.symbolTable[sectionNumber].name << "\n";
    
    uint32_t address = 0;

    const auto& code = sectionMemory.getCode();
    printMemorySegment(code, "Code", address, outFile);

    const auto& literalPool = sectionMemory.getLiteralPool();
    printMemorySegment(literalPool, "Literal Pool", address, outFile);

    outFile << "-----------------------\n";
  }
}

} // asm_core
