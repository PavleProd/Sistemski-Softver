#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace common
{

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