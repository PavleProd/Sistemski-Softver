#pragma once

#include <climits>
#include <memory>

#include <assembler/assembler.hpp>

namespace common
{

enum LiteralUsageType
{
  IMM, MEM_DIR, REG_REL
};

enum SymbolUsageType
{
  IMM, MEM_DIR, REG_REL
};

struct RelocationEntry
{
  SymbolUsageType symbolUsageType; // tip koriscenja simbola
  uint32_t offset; // offset u odnosu na trenutku sekciju gde se simbol koiristi
  uint32_t symbolTableReference; // Lokalni: sekcija iz koje je simbol, globalni: simbol
};

struct SymbolUsage
{
  SymbolUsageType symbolUsageType; // tip koriscenja simbola
  uint32_t sectionNumber; // broj sekcije u tabeli simbola gde se koristi simbol
  uint32_t sectionOffset; // offset od pocetka sekcije gde se koristi simbol
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

  void writeWord(uint32_t instruction);
  void writeBSS(uint32_t numBytes);
  uint32_t writeLiteral(uint32_t literal);

  uint32_t getSectionSize() const { return code.size() + literalPool.size(); }
  MemorySegment getSectionMemory() const;
private:
  MemorySegment code;
  MemorySegment literalPool;
};

struct AssemblerCommon
{
  using AssemblerPtr = std::unique_ptr<asm_core::Assembler>; 
  static AssemblerPtr assembler;
  static uint32_t currentSourceFileLine;
};

} // namespace common

