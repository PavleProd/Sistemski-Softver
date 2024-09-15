#pragma once

#include <emulator/emulator_structures.hpp>
#include <common/assembler_common_structures.hpp>

#include <cstdint>
#include <unordered_map>

namespace emulator_core
{
constexpr uint8_t WORD_SIZE = 4;

class Memory
{
public:

  Memory(uint64_t size);

  void init(const CodeSegments& codeSegments);
  void reset();

  uint32_t readWord(uint64_t address);
  void writeWord(uint64_t address, uint32_t word);
  void writeWordIndirect(uint64_t address, uint32_t word);

private:
  std::unordered_map<uint64_t, uint32_t> memory;
  uint64_t size;
};

class Context
{
public:
  void reset();

  void writeGpr(uint8_t index, uint32_t value);
  uint32_t readGpr(uint8_t index) const;
  uint32_t readAndIncPC();

  void incSP() { gpr[common::SP] += 4; }
  void decSP() { gpr[common::SP] -= 4; }
  void writeControl(uint8_t index, uint32_t value);
  uint32_t readControl(uint8_t index) const;

  void printState() const;
private:
  std::array<uint32_t, 16> gpr = {0};
  std::array<uint32_t, 3> control = {0};
};

} // namespace emulator_core