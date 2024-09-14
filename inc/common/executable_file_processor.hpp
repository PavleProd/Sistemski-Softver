#pragma once

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
  static std::vector<uint8_t> readFromFile(const std::string& inputFilePath);
};

} // namespace common