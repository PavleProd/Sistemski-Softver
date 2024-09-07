#include <common/assembler_common_structures.hpp>

namespace common
{

// section memory
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::writeInstruction(AssemblerInstruction instruction)
{
  uint8_t bytes[4];

  bytes[0] = static_cast<uint8_t>(instruction.oc);
  bytes[1] = ((instruction.regA & 0x0F) << 4) | (instruction.regB & 0x0F);
  bytes[2] = ((instruction.regC & 0x0F) << 4) | ((instruction.disp & 0x0F00) >> 8);
  bytes[3] = (instruction.disp & 0x00FF);

  for(int i = 0; i < 4; ++i)
  {
    code.emplace_back(bytes[i]);
  }
}
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
  return 0; // TODO upis literala u bazen
}
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::repairMemory(uint32_t start, MemorySegment repairBytes)
{
  for(int i = 0, repairSize = repairBytes.size(); i < repairSize; ++i)
  {
    code[start + i] = repairBytes[i];
  }
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
//-----------------------------------------------------------------------------------------------------------
SectionMemory::MemorySegment SectionMemory::toMemorySegment(uint32_t value)
{
  MemorySegment memorySegment;
  uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);

  for(uint32_t i = 0, numBytes = sizeof(value); i < numBytes; ++i)
  {
    memorySegment.emplace_back(bytes[i]);
  }

  return memorySegment;
}

} // namespace common

