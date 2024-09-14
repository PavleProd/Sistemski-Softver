#pragma once

#include <string>

namespace emulator_core
{

class Emulator
{
public:
  Emulator(const std::string& inputFilePath);
  void emulate();
private:
  std::string inputFilePath;
};

} // namespace emulator_core