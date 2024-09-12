#include <linker/linker.hpp>

#include <common/object_file_processor.hpp>

namespace lnk_core
{

Linker::Linker(
        const std::vector<SectionPlacement>& sectionPlacements,
        const std::vector<std::string>& inputFilePaths,
        const std::string& outputFilePath)
        : sectionPlacements(sectionPlacements), inputFilePaths(inputFilePaths), outputFilePath(outputFilePath)
{}

void Linker::performLinking()
{
  readInputFiles();
}

void Linker::readInputFiles()
{
  for(const std::string& inputFilePath : inputFilePaths)
  {
    objectFilesData.emplace_back(ObjectFileProcessor::readFromFile(inputFilePath));
  }
}

} // namespace lnk_core