#pragma once

#include <string>
#include <vector>
#include <limits>

namespace Assembler
{

// strukture asemblera

enum SymbolUsageType
{
  UNDEFINED
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
  size_t size; // SEKCIJA: velicina, SIMBOL: -1
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
  void insertGlobalSymbol(std::string symbolName);

private:
  std::vector<Symbol> symbolTable;
  uint32_t currentSectionNumber = 0; // indeks trenutne sekcije u tabeli simbola. 0 - UND
};


} // namespace Assembler

