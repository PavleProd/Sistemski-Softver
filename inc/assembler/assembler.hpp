#pragma once

#include <common/assembler_common_structures.hpp>

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

using namespace common;

using VariantType = std::variant<std::string, uint32_t, uint8_t>;

namespace asm_core
{

// instrukcije asemblera

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
  void insertBSS(uint32_t numBytes);

  void insertInstruction(InstructionTypes instructionType, const std::vector<uint8_t>& parameters);
  void insertLoadInstruction(MemoryInstructionType instructionType, const std::vector<VariantType>& parameters);

  void endAssembly();
  void printTables() const;
private:
  uint32_t findSymbol(const std::string& symbolName) const;
  void closeCurrentSection();

  void validateSymbolTable();
  void patchFromLiteralPool();
  void backpatch();
  void createRelocationTables();

  void printSymbolTable() const;
  void printRelocationTables() const;
  void printGeneratedCode() const;

  std::vector<Symbol> symbolTable;
  std::unordered_map<uint32_t, SectionMemory> sectionMemoryMap;
  std::unordered_map<uint32_t, std::vector<RelocationEntry>> sectionRelocationMap;
  std::unordered_map<uint32_t, std::vector<LiteralPoolPatch>> sectionPoolPatchesMap;

  std::string outputFilePath;
  
  uint32_t currentSectionNumber = 0; // indeks trenutne sekcije u tabeli simbola. 0 - UND
  uint32_t locationCounter = 0; // trenutna velicina generisanog koda sekcije
};

} // namespace asm_core

