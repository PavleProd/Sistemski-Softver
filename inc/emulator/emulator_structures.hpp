#pragma once

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

using CodeSegments = std::vector<CodeSegment>;

} // namespace emulator_core


