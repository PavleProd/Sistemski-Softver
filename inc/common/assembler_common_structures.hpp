#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace common
{

enum class InstructionTypes
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

enum class OperationCodes
{
  HALT = 0x00,
  INT = 0x10,

  CALL_REG_DIR = 0x20,
  CALL_REG_IND = 0x21,

  JMP_IMM = 0x30,
  BEQ_IMM = 0x31,
  BNE_IMM = 0x32,
  BGT_IMM = 0x33,
  JMP_MEM_DIR = 0x38,
  BEQ_MEM_DIR = 0x39,
  BNE_MEM_DIR = 0x3A,
  BGT_MEM_DIR = 0x3B,

  XCHG = 0x40,

  ADD = 0x50,
  SUB = 0x51,
  MUL = 0x52,
  DIV = 0x53,

  NOT = 0x60,
  AND = 0x61,
  OR = 0x62,
  XOR = 0x63,

  SHL = 0x70,
  SHR = 0x71,

  ST_MEM_DIR = 0x80,
  ST_MEM_IND = 0x81,
  ST_MEM_DIR_INC = 0x82,

  LD_CSR = 0x90,
  LD_IMM = 0x91,
  LD_MEM_DIR = 0x92,
  LD_MEM_DIR_INC = 0x93,

  LD_CSR_REG = 0x94,
  LD_CSR_OR = 0x95,
  LD_CSR_MEM_DIR = 0x96,
  LD_CSR_MEM_DIR_INC = 0x97
};

struct AssemblerInstruction
{
  OperationCodes oc; // OC + MOD, 8B
  uint8_t regA; // 4B
  uint8_t regB; // 4B
  uint8_t regC; // 4B
  uint16_t disp; // 12B
};

enum class LiteralUsageType
{
  LIT_IMM, LIT_MEM_DIR, LIT_REG_REL
};

enum class SymbolUsageType
{
  SYM_IMM, SYM_MEM_DIR, SYM_REG_REL
};

struct RelocationEntry
{
  SymbolUsageType symbolUsageType; // tip koriscenja simbola
  uint32_t offset; // offset u odnosu na trenutku sekciju gde se simbol koiristi
  uint32_t symbolTableReference; // Lokalni: sekcija iz koje je simbol, globalni: simbol

  RelocationEntry(SymbolUsageType symbolUsageType, uint32_t offset, uint32_t symbolTableReference)
    : symbolUsageType(symbolUsageType), offset(offset), symbolTableReference(symbolTableReference) {}
};

struct SymbolUsage
{
  SymbolUsageType symbolUsageType; // tip koriscenja simbola
  uint32_t sectionNumber; // broj sekcije u tabeli simbola gde se koristi simbol
  uint32_t sectionOffset; // offset od pocetka sekcije gde se koristi simbol

  SymbolUsage(SymbolUsageType symbolUsageType, uint32_t sectionNumber, uint32_t sectionOffset)
    : symbolUsageType(symbolUsageType), sectionNumber(sectionNumber), sectionOffset(sectionOffset) {}
};

struct Symbol
{
  std::string name; // ime simbola
  uint32_t sectionNumber; // SIMBOL: 0/brojTrenutneSekcije, SEKCIJA: vrednost jednaka indeksu
  int value; // SIMBOL: vrednost, SEKCIJA: UNUSED
  bool isGlobal; // SIMBOL: da li je simbol globalan (izvozimo ga) SEKCIJA: UNUSED
  bool isExtern; // SIMBOL: da li je simbol eksterni (uvozimo ga) SEKCIJA: UNUSED
  bool isDefined; // SIMBOL: da li je simbol definisan SEKCIJA: UNUSED
  uint32_t size; // SIMBOL: UNUSED, SEKCIJA: velicina
  std::vector<SymbolUsage> symbolUsages; // sva koriscenja simbola u kodu

  Symbol(const std::string& name, uint32_t sectionNumber, int value, bool isGlobal, 
                                      bool isExtern, bool isDefined, uint32_t size)
    : name(name), sectionNumber(sectionNumber), value(value), isGlobal(isGlobal), 
                            isExtern(isExtern), isDefined(isDefined), size(size) {}
};

class SectionMemory
{
public:
  using MemorySegment = std::vector<uint8_t>;

  void writeInstruction(AssemblerInstruction instruction);
  void writeWord(uint32_t instruction);
  void writeBSS(uint32_t numBytes);
  uint32_t writeLiteral(uint32_t literal);

  void repairMemory(uint32_t start, MemorySegment repairBytes);

  uint32_t getSectionSize() const { return code.size() + literalPool.size(); }
  MemorySegment getSectionMemory() const;
  
  const MemorySegment& getCode() const { return code; }
  const MemorySegment& getLiteralPool() const { return literalPool; } 

  static MemorySegment toMemorySegment(uint32_t value);
private:
  MemorySegment code;
  MemorySegment literalPool;
};

} // namespace common