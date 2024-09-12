#include <linker/linker.hpp>

#include <common/object_file_processor.hpp>
#include <common/exceptions.hpp>

#include <unordered_set>

namespace
{
  constexpr uint32_t INVALID_SECTION = 0;
}

namespace lnk_core
{

Linker::Linker(
        const std::vector<SectionPlacement>& sectionPlacements,
        const std::vector<std::string>& inputFilePaths,
        const std::string& outputFilePath)
        : sectionPlacements(sectionPlacements), inputFilePaths(inputFilePaths), outputFilePath(outputFilePath)
{}
//---------------------------------------------------------------------------------------------------------------------
void Linker::performLinking()
{
  readInputFiles();

  processProgramSections();

  if(!isPlacingSectionsPossible())
  {
    throw LinkerError("Nije moguce smestiti sekcije na navedenim adresama!");
  }
  findSectionsStartAddress();

  printGlobalSectionData();
}
//---------------------------------------------------------------------------------------------------------------------
void Linker::readInputFiles()
{
  for(const std::string& inputFilePath : inputFilePaths)
  {
    objectFilesData.emplace_back(ObjectFileProcessor::readFromFile(inputFilePath));
  }
}
//---------------------------------------------------------------------------------------------------------------------
// Spaja kod istoimenih sekcija.
// popunjava mapu sa velicinama  sekcija i vektor sa redosledom sekcija.
// Radi update tabela simbola gde ce nova vrednost simbola biti njihova adresa u memoriji
void Linker::processProgramSections()
{
  for(auto& data : objectFilesData)
  {
    for(int i = 1, tableSize = data.symbolTable.size(); i < tableSize; ++i)
    {
      Symbol& symbol = data.symbolTable[i];
      if(i != symbol.sectionNumber) // nije sekcija
      {
        continue;
      }

      const std::string& sectionName = symbol.name;
      GlobalSectionData& sectionData = globalSectionDataMap[sectionName];
      if(sectionData.size == INVALID_NUMBER) // sekcija se prvi put dodaje
      {
        sectionOrder.emplace_back(sectionName);
      }
      // adresa sekcije u odnosu na druge istoimene sekcije. Kasnije cemo dodati i pocetnu adresu sekcija
      symbol.value = sectionData.size;
      sectionData.size += symbol.size; // povecavanje velicine spojenih istoimenih sekcija
      sectionData.generatedCode.writeBytes(data.sectionMemoryMap[i]);
    }
  }
}
//---------------------------------------------------------------------------------------------------------------------
// Vraca true ako je moguce da se sekcije stave na predefinisane pozicije, inace false
bool Linker::isPlacingSectionsPossible()
{
  uint32_t numPlacements = sectionPlacements.size();
  for(int i = 0; i < numPlacements; ++i)
  {
    const SectionPlacement& placementI = sectionPlacements[i];
    uint32_t startI = placementI.startAddress;
    uint32_t endI = startI + globalSectionDataMap[placementI.sectionName].size;

    for(int j = i + 1; j < numPlacements; ++j)
    {
      const SectionPlacement& placementJ = sectionPlacements[j];
      uint32_t startJ = placementJ.startAddress;
      uint32_t endJ = startJ + globalSectionDataMap[placementJ.sectionName].size;

      if(startI < endJ && endI > startJ)
      {
        return false;
      }
    }
  }

  return true;
}
//---------------------------------------------------------------------------------------------------------------------
void Linker::findSectionsStartAddress()
{
  uint32_t firstFreeAddress = 0;

  std::unordered_set<std::string> arrangedSections; 
  for(auto& placement : sectionPlacements)
  {
    GlobalSectionData& sectionData = globalSectionDataMap[placement.sectionName];
    sectionData.startAddress = placement.startAddress;

    firstFreeAddress = std::max(sectionData.startAddress + sectionData.size, firstFreeAddress);
  }

  for(std::string& sectionName : sectionOrder)
  {
    if(arrangedSections.find(sectionName) != arrangedSections.end()) // sekcija ima predefinisanu adresu
    {
      continue;
    }

    GlobalSectionData& sectionData = globalSectionDataMap[sectionName];
    sectionData.startAddress = firstFreeAddress;
    firstFreeAddress += sectionData.size;
  }
}

void Linker::printGlobalSectionData()
{
    std::cout << "Global Section Data:\n";
    std::cout << "-----------------------------------\n";
    
    for (const std::string& sectionName : sectionOrder)
    {
        const GlobalSectionData& sectionData = globalSectionDataMap[sectionName];
        std::cout << "Section Name: " << sectionName << "\n";
        std::cout << "Start Address: " << sectionData.startAddress << "\n";
        std::cout << "Size: " << sectionData.size << " " << sectionData.generatedCode.getCodeSize() << " bytes\n";
        std::cout << "-----------------------------------\n";
    }
}

} // namespace lnk_core