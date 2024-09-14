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
emulator_core::CodeSegments ExecutableFileProcessor::readFromFile(const std::string& inputFilePath)
{
  std::ifstream inFile(inputFilePath);
  if(!inFile.is_open())
  {
    throw common::RuntimeError("Fajl na putanji " + inputFilePath + " nije mogao biti otvoren!");
  }

  uint32_t nextAddress = UINT32_MAX;
  emulator_core::CodeSegments segments;
  std::string line;
  while(std::getline(inFile, line))
  {
    if (line.empty())
    {
      continue;
    }

    uint32_t address;
    
    std::stringstream ss(line.substr(0, 4));
    ss >> std::hex >> address;

    if(address != nextAddress) // segment nije spojen, pravimo novi objekat
    {
      segments.emplace_back(address);
      nextAddress = address;
    }
    auto& segment = segments.back();

    std::vector<uint8_t> bytes;
    std::stringstream byteStream(line.substr(6));
    uint8_t byte;
    while(byteStream >> std::hex >> byte)
    {
      segment.code.emplace_back(byte);
      ++nextAddress;
    }
  }

  return segments;
}

} // namespace common