#pragma once

#include <string>
#include <vector>

namespace Assembler
{

// strukture asemblera

enum SymbolUsageType
{
  IMM, MEM_DIR, REG_DIR, REG_IND, REG_REL
};

struct SymbolUsage
{
  uint32_t sectionOffset; // offset od pocetka sekcije
  uint32_t sectionNumber; // ulaz u tabeli simbola
  SymbolUsageType symbolUsageType; // tip koriscenja simbola
};

struct Symbol
{
  std::string name; // ime simbola
  uint32_t sectionNumber; // SEKCIJA: vrednost jednaka indeksu, SIMBOL: 0
  int value; // SIMBOL: vrednost, SEKCIJA: 0
  bool isGlobal; // SIMBOL: da li je simbol globalan (izvozimo ga) SEKCIJA: false
  bool isExtern; // SIMBOL: da li je simbol eksterni (uvozimo ga) SEKCIJA: false
  uint32_t size; // SEKCIJA: velicina, SIMBOL: UINT32_MAX
  std::vector<SymbolUsage> symbolUsages; // sva koriscenja simbola u kodu
};

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
  Assembler();
  void insertGlobalSymbol(const std::string& symbolName);

private:
  uint32_t findSymbol(const std::string& symbolName) const;

  std::vector<Symbol> symbolTable;
  std::vector<uint32_t> literalPool; // bazen literala i lokalnih simbola
  
  uint32_t currentSectionNumber = 0; // indeks trenutne sekcije u tabeli simbola. 0 - UND
  uint32_t locationCounter = 0;
};


} // namespace Assembler

