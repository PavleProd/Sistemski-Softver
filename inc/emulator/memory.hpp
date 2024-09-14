#pragma once

#include <emulator/emulator_structures.hpp>

#include <cstdint>
#include <vector>

namespace emulator_core
{

class Memory
{
public:

  Memory(uint64_t size);

  void initMemory(const CodeSegments& codeSegments);
  void resetMemory();

  uint32_t readWord(uint64_t address);
  void writeWord(uint64_t address, uint32_t word);

private:
  std::vector<uint8_t> memory;
  uint64_t size;
};

} // namespace emulator_core