#include <linker/linker.hpp>
#include <linker/linker_structures.hpp>

#include <common/exceptions.hpp>
#include <common/object_file_processor.hpp>

#include <iostream>
#include <cstring>

using namespace lnk_core;
using namespace common;

int main(int argc, char* argv[])
{
  std::vector<SectionPlacement> placements;
  std::vector<std::string> inputFilePaths;
  std::string outputFilePath;
  bool hexFlag = false;
  try
  {
    int i = 1;
    while(i < argc)
    {
      std::string argument = argv[i];

      if(argument == "-o")
      {
        if(outputFilePath.empty() && i + 1 < argc)
        {
          outputFilePath = argv[++i];
        }
        else
        {
          throw RuntimeError("Greska u -o opciji");
        }
      }
      else if(argument.find("-place=") == 0)
      {
        std::string placement = argument.substr(7);
        size_t separatorPos = placement.find('@');
        if(separatorPos != std::string::npos)
        {
          std::string sectionName = placement.substr(0, separatorPos);
          std::string addressStr = placement.substr(separatorPos + 1);
          uint32_t startAddress = strtol(addressStr.c_str(), nullptr, 0);
          placements.push_back({sectionName, startAddress});
        }
        else
        {
          throw RuntimeError("Greska u -place opciji!");
        }
      }
      else if(argument == "-hex")
      {
        hexFlag = true;
      }
      else
      {
        inputFilePaths.emplace_back(argument);
      }

      ++i;
    }

    if(!hexFlag || inputFilePaths.empty() || outputFilePath.empty())
    {
      throw RuntimeError("Neka od obaveznih opcija nije navedena!");
    }

    Linker linker(placements, inputFilePaths, outputFilePath);
    linker.performLinking();
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return -1;
  }
}