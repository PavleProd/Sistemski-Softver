#pragma once

#include <linker/linker_structures.hpp>

#include <vector>
#include <string>

namespace lnk_core
{

class Linker
{

public:
  Linker(
        const std::vector<SectionPlacement>& sectionPlacements,
        const std::vector<std::string>& inputFilePaths,
        const std::string& outputFilePath);

  void performLinking();

private:
  void readInputFiles();

  std::vector<LinkerInputData> objectFilesData;

  std::vector<SectionPlacement> sectionPlacements;
  std::vector<std::string> inputFilePaths;
  std::string outputFilePath;
};

} // namespace lnk_core

