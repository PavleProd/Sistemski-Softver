#include <common/executable_file_processor.hpp>
#include <common/exceptions.hpp>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <fstream>

namespace common
{

void ExecutableFileProcessor::writeToFile(
  const std::vector<GlobalSectionData> globalSectionData, 
  const std::string& outputFilePath)
{
  std::ofstream outFile(outputFilePath);
  if(!outFile.is_open())
  {
    throw common::RuntimeError("Fajl na putanji " + outputFilePath + " nije mogao biti otvoren!");
  }

  for(const auto& data : globalSectionData)
  {
    uint32_t startAddress = data.startAddress, size = data.size;
    const auto& code = data.generatedCode.getCode();
    if(size != code.size())
    {
      throw common::LinkerError("Greska u velicini generisanog koda!");
    }

    uint32_t address = startAddress;
    for (size_t i = 0; i < size; i += 8)
    {
        outFile << std::setfill('0') << std::setw(4) << std::hex << address << ": ";
        for (size_t j = 0; j < 8 && (i + j) < code.size(); ++j)
        {
          outFile << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(code[i + j]) << " ";
        }

        outFile << "\n";
        address += 8;
    }

  }
}
//---------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ExecutableFileProcessor::readFromFile(const std::string& inputFilePath)
{

}

} // namespace common