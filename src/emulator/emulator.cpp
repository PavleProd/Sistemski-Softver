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
  switch(instruction.oc)
  {
    case OperationCodes::HALT:
      isRunning = false;
      break;
    case OperationCodes::XCHG:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regB, regC);
      context.writeGpr(instruction.regC, regB);
      break;
    }
    case OperationCodes::ADD:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regB + regC);
      break;
    }
    case OperationCodes::SUB:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regB - regC);
      break;
    }
    case OperationCodes::MUL:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regB - regC);
      break;
    }
    case OperationCodes::DIV:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      if(regC == 0)
      {
        throw EmulatorError("Pokusaj deljenja sa nulom!");
      }
      context.writeGpr(instruction.regA, regB / regC);
      break;
    }
    case OperationCodes::NOT:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      context.writeGpr(instruction.regA, ~regB);
      break;
    }
    case OperationCodes::AND:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regB & regC);
      break;
    }
    case OperationCodes::OR:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regB | regC);
      break;
    }
    case OperationCodes::XOR:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regB ^ regC);
      break;
    }
    case OperationCodes::SHL:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regB << regC);
      break;
    }
    case OperationCodes::SHR:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regB >> regC);
      break;
    }
    case OperationCodes::LD_REG_IMM:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      context.writeGpr(instruction.regA, regB + instruction.disp);
      break;
    }
    case OperationCodes::LD_REG_MEM_DIR:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      uint32_t value = memory.readWord(regB + regC + instruction.disp);
      context.writeGpr(instruction.regA, value);
      break;
    }
    default:
      context.printState();
      throw EmulatorError("Instrukcija nije prepoznata!");
  }
}
} // namespace emulator_core