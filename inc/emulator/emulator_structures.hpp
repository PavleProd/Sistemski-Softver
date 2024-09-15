#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace emulator_core
{

struct CodeSegment
{
  uint32_t startAddress;
  std::vector<uint8_t> code;

  CodeSegment (uint32_t startAddress) : startAddress(startAddress) {}
};

enum class InterruptType : uint8_t
{
  ERROR = 1,
  TIMER,
  TERMINAL,
  SOFTWARE
};

using CodeSegments = std::vector<CodeSegment>;

} // namespace emulator_core


