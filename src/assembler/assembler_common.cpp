#include <common/assembler_common.hpp>

namespace common
{

uint32_t AssemblerCommon::currentSourceFileLine = 1;
AssemblerCommon::AssemblerPtr AssemblerCommon::assembler;

// section memory
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::writeWord(uint32_t instruction)
{
  uint8_t* bytes = reinterpret_cast<uint8_t*>(&instruction);

  for(uint32_t i = 0, numBytes = sizeof(instruction); i < numBytes; ++i)
  {
    code.emplace_back(bytes[i]);
  }
}
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::writeBSS(uint32_t numBytes)
{
  for(uint32_t i = 0; i < numBytes; ++i)
  {
    code.emplace_back(0);
  }
}
//-----------------------------------------------------------------------------------------------------------
uint32_t SectionMemory::writeLiteral(uint32_t literal)
{
  return 0; // TODO
}
//-----------------------------------------------------------------------------------------------------------
SectionMemory::MemorySegment SectionMemory::getSectionMemory() const
{
  MemorySegment sectionMemory;
  sectionMemory.reserve(code.size() + literalPool.size());
  sectionMemory.insert(sectionMemory.end(), code.begin(), code.end());
  sectionMemory.insert(sectionMemory.end(), literalPool.begin(), literalPool.end());

  return sectionMemory;
}

} // namespace common