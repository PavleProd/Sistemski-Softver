#include <inc/assembler.hpp>

namespace Assembler
{

Assembler::Assembler()
{
  Symbol undefinedSection{"UND", 0, 0, false, false, 0};
  symbolTable.emplace_back(undefinedSection);
}

void Assembler::insertGlobalSymbol(std::string symbolName)
{

}

} // namespace Assembler