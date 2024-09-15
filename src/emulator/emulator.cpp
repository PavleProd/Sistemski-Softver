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
    case OperationCodes::INT:
    {
      executeInterrupt(InterruptType::SOFTWARE);
      break;
    }
    case OperationCodes::CALL_REG_DIR:
    {
      push(context.readGpr(PC));
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      context.writeGpr(PC, regA + regB + instruction.disp);
      break;
    }
    case OperationCodes::CALL_REG_IND:
    {
      push(context.readGpr(PC));
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      context.writeGpr(PC, memory.readWord(regA + regB + instruction.disp));
      break;
    }
    case OperationCodes::JMP_IMM:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      context.writeGpr(PC, regA + instruction.disp);
      break;
    }
    case OperationCodes::JMP_MEM_DIR:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      context.writeGpr(PC, memory.readWord(regA + instruction.disp));
      break;
    }
    case OperationCodes::BEQ_IMM:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      if(regB == regC)
      {
        context.writeGpr(PC, regA + instruction.disp);
      }
      break;
    }
    case OperationCodes::BEQ_MEM_DIR:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      if(regB == regC)
      {
        context.writeGpr(PC, memory.readWord(regA + instruction.disp));
      }
      break;
    }
    case OperationCodes::BNE_IMM:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      if(regB != regC)
      {
        context.writeGpr(PC, regA + instruction.disp);
      }
      break;
    }
    case OperationCodes::BNE_MEM_DIR:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      if(regB != regC)
      {
        context.writeGpr(PC, memory.readWord(regA + instruction.disp));
      }
      break;
    }
    case OperationCodes::BGT_IMM:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      if(static_cast<int>(regB) > static_cast<int>(regC))
      {
        context.writeGpr(PC, regA + instruction.disp);
      }
      break;
    }
    case OperationCodes::BGT_MEM_DIR:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      if(static_cast<int>(regB) > static_cast<int>(regC))
      {
        context.writeGpr(PC, memory.readWord(regA + instruction.disp));
      }
      break;
    }
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
    case OperationCodes::ST_MEM_DIR:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      memory.writeWord(regA + regB + instruction.disp, regC);
    }
    case OperationCodes::ST_MEM_IND:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      memory.writeWordIndirect(regA + regB + instruction.disp, regC);
      break;
    }
    case OperationCodes::ST_MEM_DIR_INC:
    {
      uint32_t regA = context.readGpr(instruction.regA);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeGpr(instruction.regA, regA + instruction.disp);
      regA = context.readGpr(instruction.regA);
      memory.writeWord(regA, regC);
      break;
    }
    case OperationCodes::LD_REG_IMM:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      context.writeGpr(instruction.regA, regB + instruction.disp);
      break;
    }
    case OperationCodes::LD_REG_CSR:
    {
      uint32_t csrB = context.readControl(instruction.regB);
      context.writeGpr(instruction.regA, csrB);
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
    case OperationCodes::LD_REG_MEM_DIR_INC:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      context.writeGpr(instruction.regA, memory.readWord(regB));
      context.writeGpr(instruction.regB, regB + static_cast<int>(instruction.disp));
      break;
    }
    case OperationCodes::LD_CSR_REG:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      context.writeControl(instruction.regA, regB);
      break;
    }
    case OperationCodes::LD_CSR_OR:
    {
      uint32_t csrB = context.readControl(instruction.regB);
      context.writeControl(instruction.regA, csrB | instruction.disp);
      break;
    }
    case OperationCodes::LD_CSR_MEM_DIR:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      uint32_t regC = context.readGpr(instruction.regC);
      context.writeControl(instruction.regA, memory.readWord(regB + regC + instruction.disp));
      break;
    }
    case OperationCodes::LD_CSR_MEM_DIR_INC:
    {
      uint32_t regB = context.readGpr(instruction.regB);
      context.writeControl(instruction.regA, memory.readWord(regB));
      context.writeGpr(instruction.regB, regB + static_cast<int>(instruction.disp));
      break;
    }
    default:
      context.printState();
      executeInterrupt(InterruptType::ERROR);
      throw EmulatorError("Instrukcija nije prepoznata!");
  }
}
//-----------------------------------------------------------------------------------------------------------
void Emulator::executeInterrupt(InterruptType interruptType)
{
  push(context.readControl(STATUS));
  push(context.readGpr(PC));
  context.writeControl(CAUSE, static_cast<uint8_t>(interruptType));
  context.writeControl(STATUS, context.readControl(STATUS) & (~0x1));
  context.writeGpr(PC, context.readControl(HANDLER));
}
//-----------------------------------------------------------------------------------------------------------
void Emulator::push(uint32_t value)
{
  context.decSP();
  memory.writeWord(context.readGpr(SP), value);
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Emulator::pop()
{
  uint32_t value = memory.readWord(context.readGpr(SP));
  context.incSP();
  return value;
}

} // namespace emulator_core