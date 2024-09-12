#pragma once

#include <common/assembler_common_structures.hpp>

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

using namespace common;

using ParameterType = std::variant<std::string, uint32_t, uint8_t, uint16_t>;
using Parameters = std::vector<ParameterType>;

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

  void insertLoadInstructionRegister(MemoryInstructionType instructionType, const Parameters& parameters);
  void insertLoadInstructionLiteral(MemoryInstructionType instructionType, const Parameters& parameters);
  void insertLoadInstructionSymbol(MemoryInstructionType instructionType, const Parameters& parameters);

  void insertStoreInstructionRegister(MemoryInstructionType instructionType, const Parameters&& parameters);
  void insertStoreInstructionLiteral(MemoryInstructionType instructionType, const Parameters&& parameters);
  void insertStoreInstructionSymbol(MemoryInstructionType instructionType, const Parameters& parameters);

  void insertJumpInstructionLiteral(InstructionTypes instructionType, const Parameters&& parameters);
  void insertJumpInstructionSymbol(InstructionTypes instructionType, const Parameters&& parameters);

  void endAssembly();
private:
  uint32_t findSymbol(const std::string& symbolName) const;
  uint32_t findPoolOffset(uint32_t symbolIndex) const;
  void closeCurrentSection();

  void validateSymbolTable();
  void patchFromLiteralPool();
  void backpatch();
  void createRelocationTables();

  std::vector<Symbol> symbolTable;
  std::vector<std::string> sectionOrder;
  std::unordered_map<uint32_t, SectionMemory> sectionMemoryMap;
  std::unordered_map<uint32_t, std::vector<RelocationEntry>> sectionRelocationMap;
  std::unordered_map<uint32_t, std::vector<LiteralPoolPatch>> sectionPoolPatchesMap;

  std::string outputFilePath;
  
  uint32_t currentSectionNumber = 0; // indeks trenutne sekcije u tabeli simbola. 0 - UND
  uint32_t locationCounter = 0; // trenutna velicina generisanog koda sekcije
};

} // namespace asm_core

