#include <assembler/assembler.hpp>
#include <common/exceptions.hpp>

constexpr uint32_t INVALID = 0;
constexpr uint32_t UNUSED = UINT32_MAX;
constexpr int UNUSED_INT = -1;

namespace asm_core
{

Assembler::Assembler()
{
  Symbol undefinedSection{"UND", 0, UNUSED_INT, false, false, 0};
  symbolTable.emplace_back(undefinedSection);
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertGlobalSymbol(const std::string& symbolName)
{
  const uint32_t symbolIndex = findSymbol(symbolName);
  
  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolTable.emplace_back(symbolName, INVALID, INVALID, true, false, UNUSED);
  }
  else // jeste u tabeli simbola
  {
    Symbol& symbol = symbolTable[symbolIndex];
    
    if(symbol.isExtern)
    {
      throw common::CustomError(common::ErrorCode::GLOBAL_EXTERN_CONFLICT);
    }

    symbol.isGlobal = true;
  }
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Assembler::findSymbol(const std::string& symbolName) const
{
  for(uint32_t i = 0, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    if(symbolTable[i].name == symbolName)
    {
      return i;
    }
  }

  return INVALID;
}

} // namespace asm_core