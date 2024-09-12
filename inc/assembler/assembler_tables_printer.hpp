#pragma once

#include <common/assembler_common_structures.hpp>

#include <string>
#include <fstream>


namespace asm_core
{

class AssemblerTablesPrinter
{
public:
  static void printTables(const common::AssemblerOutputData& data, const std::string& filePath);
private:
  static void printSymbolTable(const std::vector<common::Symbol>& symbolTable, std::ofstream& outFile);
  static void printRelocationTables(const common::AssemblerOutputData& data, std::ofstream& outFile);
  static void printGeneratedCode(const common::AssemblerOutputData& data, std::ofstream& outFile);
};

} // asm_core