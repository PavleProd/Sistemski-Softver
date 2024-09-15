#include <emulator/emulator.hpp>
#include <common/exceptions.hpp>
#include <common/executable_file_processor.hpp>

#include <iostream>

namespace
{
constexpr uint64_t DEFAULT_MEMORY_SIZE = (1ULL << 32);
} // unnamed

namespace emulator_core
{

Emulator::Emulator(const std::string& inputFilePath)
  : inputFilePath(inputFilePath), memory(DEFAULT_MEMORY_SIZE) {}
//-----------------------------------------------------------------------------------------------------------
void Emulator::emulate()
{
  const auto& segments = ExecutableFileProcessor::readFromFile(inputFilePath);
  memory.init(segments);
  context.reset();
  while(isRunning)
  {
    uint32_t instruction = memory.readWord(context.readAndIncPC());
    executeInstruction(toInstruction(instruction));
  }

  std::cout << "Izvrsavanje zaustavljeno HALT instrukcijom!\n";
  context.printState();
}
//-----------------------------------------------------------------------------------------------------------
AssemblerInstruction Emulator::toInstruction(uint32_t word)
{
  AssemblerInstruction instruction;
  instruction.oc = static_cast<OperationCodes>((word >> 24) & 0xFF); // 8B
  instruction.regA = (word >> 20) & 0x0F; // 4B
  instruction.regB = (word >> 16) & 0x0F; // 4B
  instruction.regC = (word >> 12) & 0x0F; // 4B
  instruction.disp = word & 0x0FFF; // 12B

  return instruction;
}
//-----------------------------------------------------------------------------------------------------------
void Emulator::executeInstruction(const AssemblerInstruction& instruction)
{
  std::cout << std::to_string(static_cast<int>(instruction.oc)) << "\n";
  switch(instruction.oc)
  {
    case OperationCodes::HALT:
      isRunning = false;
      break;

    default:
      context.printState();
      throw EmulatorError("Instrukcija nije prepoznata!");
  }
}
} // namespace emulator_core