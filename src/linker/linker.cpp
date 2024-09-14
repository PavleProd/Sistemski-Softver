#include <linker/linker.hpp>

#include <common/executable_file_processor.hpp>
#include <common/object_file_processor.hpp>
#include <common/exceptions.hpp>

#include <algorithm>
#include <unordered_set>

namespace
{
constexpr uint32_t INVALID_SECTION = 0;

std::vector<lnk_core::GlobalSectionData> toVector(
  std::unordered_map<std::string, lnk_core::GlobalSectionData> globalSectionDataMap)
{
  using namespace lnk_core;

  std::vector<GlobalSectionData> sectionVector;
  for (const auto& [_, globalSectionData] : globalSectionDataMap)
  {
    sectionVector.emplace_back(globalSectionData);
  }

  std::sort(sectionVector.begin(), sectionVector.end(),
    [](const GlobalSectionData& data1, const GlobalSectionData& data2)
    {
      return data1.startAddress < data2.startAddress;
    });

  return sectionVector;
}

} // namespace

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
  initGlobalSymbolTable();
  patchRelocationEntries();

  // kraj linkovanja
  ExecutableFileProcessor::writeToFile(toVector(globalSectionDataMap), outputFilePath);
  printLinkingInfo();
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
        sectionData.size = 0;
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
    firstFreeAddress = std::max(firstFreeAddress, sectionData.startAddress + sectionData.size);

    arrangedSections.insert(placement.sectionName);

    std::cout << placement.sectionName << " " << placement.startAddress << "\n";
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
//---------------------------------------------------------------------------------------------------------------------
void Linker::initGlobalSymbolTable()
{
  for(const auto& data : objectFilesData)
  {
    for(int i = 0, tableSize = data.symbolTable.size(); i < tableSize; ++i)
    {
      const Symbol& symbol = data.symbolTable[i];
      
      // obradjujemo samo simbole koje izvozimo iz tabele simbola
      if(symbol.sectionNumber == i || symbol.isExtern || !symbol.isGlobal)
      {
        continue;
      }

      if(globalSymbolTable.find(symbol.name) != globalSymbolTable.end())
      {
        throw LinkerError("Redeklaracija simbola " + symbol.name);
      }

      const Symbol& section = data.symbolTable[symbol.sectionNumber];
      const auto& globalSectionData = globalSectionDataMap[section.name];

      // pocetna adresa segmenta sekcija + pocetak dela te sekcije za ovaj fajl 
      uint32_t sectionOffset = globalSectionData.startAddress + section.value;
      globalSymbolTable[symbol.name] = symbol.value + sectionOffset;
    }  
  }
}
//---------------------------------------------------------------------------------------------------------------------
void Linker::patchRelocationEntries()
{
  for(const auto& data : objectFilesData)
  {
    for(const auto& [sectionUsageNumber, relocationEntries] : data.sectionRelocationMap)
    {
      for(const RelocationEntry& entry : relocationEntries)
      {
        // oc za trenutni skup instrukcija nije bitan (sve upisujemo na 4B)
        const Symbol& reference = data.symbolTable[entry.symbolTableReference];

        uint32_t symbolValue;
        if(entry.symbolTableReference == reference.sectionNumber) // sekcija
        {
          const auto& globalSectionData = globalSectionDataMap[reference.name];
          symbolValue = globalSectionData.startAddress + reference.value;
        }
        else // globalni simbol
        {
          if(globalSymbolTable.find(reference.name) == globalSymbolTable.end())
          {
            throw LinkerError("Upotrebljen nepostojeci simbol " + reference.name);
          }
          symbolValue = globalSymbolTable[reference.name];
        }

        const Symbol& sectionUsage = data.symbolTable[sectionUsageNumber];

        // mem[sectionUsageStartAddr + offset] = symbolValue
        auto& globalSectionData = globalSectionDataMap[sectionUsage.name];
        // memorija je samo za trenutne sekcije sa ovim imenom pa uzimamo offset za sekciju u ovom fajlu + offset do koriscenja
        uint32_t fixAddr = sectionUsage.value + entry.offset;
        globalSectionData.generatedCode.addToAddress(fixAddr, symbolValue);
      }
    }

  }
}
//---------------------------------------------------------------------------------------------------------------------
void Linker::printLinkingInfo()
{
  printGlobalSectionData();
  printGlobalSymbolTable();
}
//---------------------------------------------------------------------------------------------------------------------
void Linker::printGlobalSectionData()
{
    std::cout << "===================================\n";
    std::cout << "Global Section Data:\n";
    
    for (const std::string& sectionName : sectionOrder)
    {
        const GlobalSectionData& sectionData = globalSectionDataMap[sectionName];
        std::cout << "-----------------------------------\n";
        std::cout << "Section Name: " << sectionName << "\n";
        std::cout << "Start Address: " << sectionData.startAddress << "\n";
        std::cout << "Size: " << std::dec << sectionData.size << " bytes\n";
    }
    std::cout << "===================================\n";
}

void Linker::printGlobalSymbolTable()
{
  std::cout << "Global Symbol Table:\n";
  std::cout << "-----------------------------------\n";
  std::cout << "Name,Value\n";
  for(const auto& [symbolName, value] : globalSymbolTable)
  {
    std::cout << symbolName << ", 0x" << std::hex << value << "\n";
  }
  std::cout << "===================================\n";
}

} // namespace lnk_core