#pragma once

#include <common/assembler_common_structures.hpp>
#include <linker/linker_structures.hpp>

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

using namespace common;

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
  void processProgramSections();

  bool isPlacingSectionsPossible();
  void findSectionsStartAddress();

  void printGlobalSectionData();

  std::vector<LinkerInputData> objectFilesData;
  
  std::unordered_map<std::string, GlobalSectionData> globalSectionDataMap;
  std::vector<std::string> sectionOrder;

  std::vector<SectionPlacement> sectionPlacements;
  std::vector<std::string> inputFilePaths;
  std::string outputFilePath;
};

} // namespace lnk_core

