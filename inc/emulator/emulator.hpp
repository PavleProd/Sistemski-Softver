#pragma once

#include <common/assembler_common_structures.hpp>
#include <emulator/memory.hpp>

#include <string>
#include <unordered_map>

using namespace common;

namespace emulator_core
{

class Emulator
{
public:
  Emulator(const std::string& inputFilePath);
  void emulate();
private:
  static AssemblerInstruction toInstruction(uint32_t word);
  void executeInstruction(const AssemblerInstruction& instruction);

  Memory memory;
  Context context;
  std::string inputFilePath;

  bool isRunning = true;
};

} // namespace emulator_core