#pragma once

#include <climits>
#include <memory>

#include <inc/assembler/assembler.hpp>

namespace asm_core
{
  std::unique_ptr<Assembler> assembler;
  uint32_t currentSourceFileLine = 0;
} // namespace asm_core