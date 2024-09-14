#pragma once

#include <emulator/memory.hpp>

#include <string>

namespace emulator_core
{

class Emulator
{
public:
  Emulator(const std::string& inputFilePath);
  void emulate();
private:
  Memory memory;
  std::string inputFilePath;
};

} // namespace emulator_core