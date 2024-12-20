#include <common/assembler_common_structures.hpp>
#include <common/exceptions.hpp>

#include <iostream>

namespace common
{

// section memory
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::writeInstruction(AssemblerInstruction instruction)
{
  const MemorySegment& memorySegment = toMemorySegment(instruction);

  for(int i = 0; i < 4; ++i)
  {
    code.emplace_back(memorySegment[i]);
  }
}
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::writeWord(uint32_t instruction)
{
  uint8_t* bytes = reinterpret_cast<uint8_t*>(&instruction);
  for(int i = 0, numBytes = sizeof(instruction); i < numBytes; ++i)
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
void SectionMemory::writeBytes(const MemorySegment& bytes)
{
  code.insert(code.end(), bytes.begin(), bytes.end());
}
//-----------------------------------------------------------------------------------------------------------
uint32_t SectionMemory::writeLiteral(uint32_t literal)
{
  uint32_t location = literalPool.size();
  uint8_t* bytes = reinterpret_cast<uint8_t*>(&literal);
  for(int i = 0, numBytes = sizeof(literal); i < numBytes; ++i)
  {
    literalPool.emplace_back(bytes[i]);
  }

  return location;
}
//-----------------------------------------------------------------------------------------------------------
uint32_t SectionMemory::readCode(uint32_t address) const
{
  if(address + 4 > code.size())
  {
    std::string message = "address=" + std::to_string(address) + ", codeSize=" + std::to_string(code.size());
    throw common::MemoryError("SectionMemory::readCode", message);
  }
  uint32_t value = 0;
  value |= code[address];
  value |= static_cast<uint32_t>(code[address + 1]) << 8;
  value |= static_cast<uint32_t>(code[address + 2]) << 16;
  value |= static_cast<uint32_t>(code[address + 3]) << 24;

  return value;
}
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::addToAddress(uint32_t address, uint32_t value)
{
  uint32_t memoryValue = readCode(address);
  repairMemory(address, toMemorySegment(memoryValue + value));
}
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::repairMemory(uint32_t start, MemorySegment repairBytes)
{
  if(start + repairBytes.size() > code.size())
  {
    std::string message = "address=" + std::to_string(start) + ", codeSize=" + std::to_string(code.size());
    throw common::MemoryError("SectionMemory::repairMemory", message);
  }

  for(int i = 0, repairSize = repairBytes.size(); i < repairSize; ++i)
  {
    code[start + i] = repairBytes[i];
  }
}
//-----------------------------------------------------------------------------------------------------------
void SectionMemory::repairLiteralPool(uint32_t start, MemorySegment repairBytes)
{
  for(int i = 0, repairSize = repairBytes.size(); i < repairSize; ++i)
  {
    literalPool[start + i] = repairBytes[i];
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

  for(int i = 0, numBytes = sizeof(value); i < numBytes; ++i)
  {
    memorySegment.emplace_back(bytes[i]);
  }

  return memorySegment;
}

SectionMemory::MemorySegment SectionMemory::toMemorySegment(AssemblerInstruction instruction)
{
  MemorySegment memorySegment;
  // little-endian format
  memorySegment.emplace_back((instruction.disp & 0x00FF));
  memorySegment.emplace_back(((instruction.regC & 0x0F) << 4) | ((instruction.disp & 0x0F00) >> 8));
  memorySegment.emplace_back(((instruction.regA & 0x0F) << 4) | (instruction.regB & 0x0F));
  memorySegment.emplace_back(static_cast<uint8_t>(instruction.oc));

  return memorySegment;
}

} // namespace common

