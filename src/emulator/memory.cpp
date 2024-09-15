#include <emulator/memory.hpp>
#include <common/exceptions.hpp>

#include <iostream>
#include <iomanip>
#include <string_view>

namespace
{

constexpr const std::string_view MEMORY_OVERFLOW = "Pokusaj upisa na lokaciju vecu od velicine memorije!";
constexpr const std::string_view INVALID_REGISTER = "Pokusaj pristupu nepostojecem registru!";
constexpr uint8_t NUM_GPR = 16;
constexpr uint8_t NUM_CONTROL = 3;

} // namespace

namespace emulator_core
{

Memory::Memory(uint64_t size)
  : size(size) {}
//-----------------------------------------------------------------------------------------------------------
void Memory::init(const CodeSegments& codeSegments)
{
  reset();
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
void Memory::reset()
{
  memory.clear();
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
//-----------------------------------------------------------------------------------------------------------
void Memory::writeWordIndirect(uint64_t address, uint32_t word)
{
  uint32_t indirectAddress = readWord(address);
  return writeWord(indirectAddress, word);
}
//-----------------------------------------------------------------------------------------------------------
void Context::reset()
{
  gpr[SP] = 0; // poslednja zauzeta ?
  gpr[PC] = 0x40000000;
}
//-----------------------------------------------------------------------------------------------------------
void Context::writeGpr(uint8_t index, uint32_t value)
{
  if(index == 0)
  {
    return;
  }

  if(index > NUM_GPR)
  {
    throw common::MemoryError("Memory::writeGpr", std::string(INVALID_REGISTER));
  }

  gpr[index] = value;
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Context::readGpr(uint8_t index) const
{
  if(index > NUM_GPR)
  {
    throw common::MemoryError("Memory::writeGpr", std::string(INVALID_REGISTER));
  }

  return gpr[index];
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Context::readAndIncPC()
{
  uint32_t pc = gpr[PC];
  gpr[PC] += 4;
  return pc;
}
//-----------------------------------------------------------------------------------------------------------
void Context::writeControl(uint8_t index, uint32_t value)
{
  if(index > NUM_CONTROL)
  {
    throw common::MemoryError("Memory::readControl", std::string(INVALID_REGISTER));
  }

  control[index] = value;
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Context::readControl(uint8_t index) const
{
  if(index > NUM_CONTROL)
  {
    throw common::MemoryError("Memory::readControl", std::string(INVALID_REGISTER));
  }

  return control[index];
}
//-----------------------------------------------------------------------------------------------------------
void Context::printState() const
{
  std::cout << "STANJE PROCESORA:\n";
  for(uint8_t i = 0; i < NUM_GPR; ++i)
  {
    std::cout << "r" << std::dec << static_cast<int>(i);
    std::cout  << "=0x" << std::hex << std::setfill('0') << std::setw(8) << gpr[i];
    std::cout << ((i + 1) % 4 == 0 ? "\n" : " ");
  }
}
} // namespace emulator_core