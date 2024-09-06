#pragma once

#include <climits>
#include <memory>

#include <assembler/assembler.hpp>

namespace common
{

struct AssemblerCommon
{
  using AssemblerPtr = std::unique_ptr<asm_core::Assembler>; 
  static AssemblerPtr assembler;
  static uint32_t currentSourceFileLine;
};

} // namespace common

