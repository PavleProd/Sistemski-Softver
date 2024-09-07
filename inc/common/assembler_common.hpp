#pragma once

#include <assembler/assembler.hpp>

#include <cstdint>

namespace common
{

struct AssemblerCommon
{
  static asm_core::Assembler* assembler;
  static uint32_t currentSourceFileLine;
};

} // namespace common

