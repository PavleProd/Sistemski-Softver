#include <common/assembler_common.hpp>

namespace common
{
  uint32_t AssemblerCommon::currentSourceFileLine = 1;
  AssemblerCommon::AssemblerPtr AssemblerCommon::assembler;
}