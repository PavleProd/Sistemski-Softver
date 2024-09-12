#include <assembler/assembler_common.hpp>

namespace common
{

uint32_t AssemblerCommon::currentSourceFileLine = 1;
asm_core::Assembler* AssemblerCommon::assembler = nullptr;

} // namespace common