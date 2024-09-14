#pragma once

#include <emulator/emulator_structures.hpp>
#include <linker/linker_structures.hpp>

#include <vector>
#include <sstream>

namespace common
{

using namespace lnk_core;

class ExecutableFileProcessor
{
public:
  static void writeToFile(const std::vector<GlobalSectionData> globalSectionData, const std::string& outputFilePath);
  static emulator_core::CodeSegments readFromFile(const std::string& inputFilePath);
};

} // namespace common