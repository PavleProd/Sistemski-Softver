#include <emulator/memory.hpp>
#include <common/exceptions.hpp>

#include <string_view>

namespace
{

constexpr const std::string_view MEMORY_OVERFLOW = "Pokusaj upisa na lokaciju vecu od velicine memorije!";
constexpr uint8_t WORD_SIZE = 4;

} // namespace

namespace emulator_core
{

Memory::Memory(uint64_t size /* = (1 << 32) */)
  : size(size) {}
//-----------------------------------------------------------------------------------------------------------
void Memory::initMemory(const CodeSegments& codeSegments)
{
  resetMemory();
  for(const auto& codeSegment : codeSegments)
  {
    const auto& code = codeSegment.code;
    if(codeSegment.startAddress + code.size() > size)
    {
      throw common::MemoryError("Memory::initMemory", std::string(MEMORY_OVERFLOW));
    }

    for(int i = 0, codeSize = code.size(); i < codeSize; ++i)
    {
      memory[codeSegment.startAddress + i] = code[i];
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
void Memory::resetMemory()
{
  memory = std::vector<uint8_t>(size, 0);
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Memory::readWord(uint64_t address)
{
  if(address + WORD_SIZE > size)
  {
    throw common::MemoryError("Memory::readWord", std::string(MEMORY_OVERFLOW));
  }

  uint32_t value = 0;
  value |= memory[address];
  value |= static_cast<uint32_t>(memory[address + 1]) << 8;
  value |= static_cast<uint32_t>(memory[address + 2]) << 16;
  value |= static_cast<uint32_t>(memory[address + 3]) << 24;

  return value;
}
//-----------------------------------------------------------------------------------------------------------
void Memory::writeWord(uint64_t address, uint32_t word)
{
  if(address + WORD_SIZE > size)
  {
    throw common::MemoryError("Memory::writeWord", std::string(MEMORY_OVERFLOW));
  }

  uint8_t* bytes = reinterpret_cast<uint8_t*>(&word);
  for(uint32_t i = 0, numBytes = sizeof(word); i < numBytes; ++i)
  {
    memory[address + i] = bytes[i];
  }
}

} // namespace emulator_core