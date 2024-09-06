#pragma once

#include <string>
#include <vector>

namespace asm_core
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
  void openNewSection(const std::string& sectionName);
  void endAssembly();

  void printTables();
private:
  uint32_t findSymbol(const std::string& symbolName) const;
  void closeCurrentSection();

  std::vector<Symbol> symbolTable;
  std::vector<uint32_t> literalPool; // bazen literala i lokalnih simbola

  std::string outputFilePath;
  
  uint32_t currentSectionNumber = 0; // indeks trenutne sekcije u tabeli simbola. 0 - UND
  uint32_t locationCounter = 0;
};


} // namespace asm_core

