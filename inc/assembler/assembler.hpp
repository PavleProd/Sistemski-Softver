#pragma once

#include <common/assembler_common.hpp>

#include <string>
#include <vector>
#include <unordered_map>

using namespace common;

namespace asm_core
{

// instrukcije asemblera

enum directives
{
  GLOBAL,
  EXTERN,
  SECTION,
  WORD,
  SKIP,
  END
};

enum instructions
{
  HALT, INT, IRET,
  CALL, RET, JMP,
  BEQ, BNE, BGT,
  PUSH, POP, XCHG,
  ADD, SUB, MUL,
  DIV, NOT, AND,
  OR, XOR, SHL,
  SHR, LD, ST,
  CSRRD, CSRWR
};

class Assembler
{
public:
  Assembler(const std::string& outputFilePath);
  void insertGlobalSymbol(const std::string& symbolName);
  void insertExternSymbol(const std::string& symbolName);
  void defineSymbol(const std::string& symbolName);

  void openNewSection(const std::string& sectionName);

  void insertSymbol(const std::string& symbolName);
  void insertLiteral(uint32_t value);

  void endAssembly();
  void printTables();
private:
  uint32_t findSymbol(const std::string& symbolName) const;
  void closeCurrentSection();

  void backpatch();
  void createRelocationTables();

  std::vector<Symbol> symbolTable;
  std::unordered_map<uint32_t, SectionMemory> sectionMemoryMap;
  std::unordered_map<uint32_t, std::vector<RelocationEntry>> sectionRelocationMap;

  std::string outputFilePath;
  
  uint32_t currentSectionNumber = 0; // indeks trenutne sekcije u tabeli simbola. 0 - UND
  uint32_t locationCounter = 0; // trenutna velicina generisanog koda sekcije
};




} // namespace asm_core

