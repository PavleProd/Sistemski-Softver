#include <assembler/assembler.hpp>
#include <assembler/assembler_tables_printer.hpp>

#include <assembler/assembler_common.hpp>
#include <common/exceptions.hpp>
#include <common/object_file_processor.hpp>

#include <cmath>
#include <iostream>
#include <iomanip>

namespace
{
constexpr uint32_t INVALID = 0;
constexpr uint32_t UNUSED = UINT32_MAX;
constexpr int UNUSED_INT = -1;

constexpr int WORD_SIZE = 4;
constexpr uint32_t VALUE_OVERFLOW_LIMIT = (1 << 13);

} // unnamed

namespace asm_core
{

Assembler::Assembler(const std::string& outputFilePath)
  : outputFilePath(outputFilePath)
{
  Symbol undefinedSection{"UND", 0, UNUSED_INT, false, false, false, 0};
  symbolTable.emplace_back(undefinedSection);
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertGlobalSymbol(const std::string& symbolName)
{
  const uint32_t symbolIndex = findSymbol(symbolName);
  
  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolTable.emplace_back(symbolName, INVALID, 0, true, false, false, UNUSED);
  }
  else // jeste u tabeli simbola
  {
    Symbol& symbol = symbolTable[symbolIndex];
    
    if(symbol.isExtern)
    {
      throw AssemblerError(ErrorCode::GLOBAL_EXTERN_CONFLICT);
    }

    symbol.isGlobal = true;
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertExternSymbol(const std::string& symbolName)
{
  const uint32_t symbolIndex = findSymbol(symbolName);

  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolTable.emplace_back(symbolName, INVALID, INVALID, false, true, true, UNUSED);
  }
  else // jeste u tabeli simbola
  {
    Symbol& symbol = symbolTable[symbolIndex];

    if(symbol.isDefined)
    {
      throw AssemblerError(ErrorCode::SYMBOL_REDECLARATION);
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::defineSymbol(const std::string& symbolName)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  uint32_t symbolIndex = findSymbol(symbolName);

  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolTable.emplace_back(symbolName, currentSectionNumber, locationCounter, false, false, true, UNUSED);
  }
  else // jeste u tabeli simbola
  {
    Symbol& symbol = symbolTable[symbolIndex];
    if(symbol.isDefined || symbol.isExtern)
    {
      throw AssemblerError(ErrorCode::SYMBOL_REDECLARATION);
    }
    
    symbol.sectionNumber = currentSectionNumber;
    symbol.isDefined = true;
    symbol.value = locationCounter;
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::openNewSection(const std::string& sectionName)
{
  const uint32_t symbolIndex = findSymbol(sectionName);

  if (symbolIndex != INVALID) // jeste u tabeli simbola, GRESKA
  {
    Symbol& symbol = symbolTable[symbolIndex];

    if(symbol.sectionNumber == symbolIndex) // redeklaracija sekcije
    {
      throw AssemblerError(ErrorCode::SECTION_REDECLARATION);
    }
    else // konflikt simbola i sekcije
    {
      throw AssemblerError(ErrorCode::SYMBOL_REDECLARATION);
    }
  }
  
  sectionOrder.emplace_back(sectionName);
  closeCurrentSection();

  currentSectionNumber = symbolTable.size();
  symbolTable.emplace_back(sectionName, currentSectionNumber, INVALID, false, false, false, 0);

}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertSymbol(const std::string& symbolName)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  uint32_t symbolIndex = findSymbol(symbolName);
  if(symbolIndex == INVALID) // nije u tabeli simbola
  {
    symbolIndex = symbolTable.size();
    symbolTable.emplace_back(symbolName, INVALID, INVALID, false, false, false, UNUSED);
  }

  // ubacujemo u niz koriscenja
  Symbol& symbol = symbolTable[symbolIndex];
  AssemblerInstruction instruction {OperationCodes::WORD, 0, 0, 0, 0}; // simbol je cela rec pa nam je to jedino bitno
  symbol.symbolUsages.emplace_back(instruction, currentSectionNumber, locationCounter);

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  sectionMemory.writeBSS(WORD_SIZE); // popunjavamo nulama, pa cemo u backpatchingu da popunimo vrednoscu simbola

  locationCounter += WORD_SIZE;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertLiteral(uint32_t value)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  sectionMemory.writeWord(value);

  locationCounter += WORD_SIZE;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertBSS(uint32_t numBytes)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  sectionMemory.writeBSS(numBytes);

  locationCounter += numBytes;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertInstruction(InstructionTypes instruction, const std::vector<uint8_t>& pars)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];

  switch(instruction)
  {
    case InstructionTypes::HALT:
      sectionMemory.writeInstruction({OperationCodes::HALT, 0, 0, 0, 0});
      break;
    case InstructionTypes::INT:
      sectionMemory.writeInstruction({OperationCodes::INT, 0, 0, 0, 0});
      break;
    case InstructionTypes::IRET:
      insertInstruction(InstructionTypes::POP, {PC}); // pop pc
      insertLoadInstructionRegister(
        MemoryInstructionType::CSR_MEM_DIR_INC,
        {static_cast<uint8_t>(SP), static_cast<uint8_t>(STATUS), static_cast<uint8_t>(WORD_SIZE)}); // pop status
        return; // ne zelimo da uvecavamo instrukciju, to rade pozivi funkcije
    case InstructionTypes::RET:
      insertInstruction(InstructionTypes::POP, {PC}); // pop pc
      return; // ne zelimo da uvecavamo instrukciju, to rade pozivi funkcije

    case InstructionTypes::PUSH:
      // SP = SP - 4, mem[SP] = reg(0)
      insertStoreInstructionRegister(
        MemoryInstructionType::REG_MEM_DIR_INC,
        {pars[0], static_cast<uint8_t>(SP), static_cast<uint8_t>(-WORD_SIZE)});
      return; // ne zelimo da uvecavamo instrukciju, to rade pozivi funkcije

    case InstructionTypes::POP:
      // reg(0) = mem[SP], SP = SP + 4
      insertLoadInstructionRegister(
        MemoryInstructionType::REG_MEM_DIR_INC,
        {static_cast<uint8_t>(SP), pars[0], static_cast<uint8_t>(WORD_SIZE)});
      return; // ne zelimo da uvecavamo instrukciju, to rade pozivi funkcije

    case InstructionTypes::XCHG:
      sectionMemory.writeInstruction({OperationCodes::XCHG, 0, pars[1], pars[0], 0});
      break;
    case InstructionTypes::ADD:
      sectionMemory.writeInstruction({OperationCodes::ADD, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::SUB:
      sectionMemory.writeInstruction({OperationCodes::SUB, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::MUL:
      sectionMemory.writeInstruction({OperationCodes::MUL, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::DIV:
      sectionMemory.writeInstruction({OperationCodes::DIV, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::NOT:
      sectionMemory.writeInstruction({OperationCodes::NOT, pars[0], pars[0], 0, 0});
      break;
    case InstructionTypes::AND:
      sectionMemory.writeInstruction({OperationCodes::AND, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::OR:
      sectionMemory.writeInstruction({OperationCodes::OR, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::XOR:
      sectionMemory.writeInstruction({OperationCodes::XOR, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::SHL:
      sectionMemory.writeInstruction({OperationCodes::SHL, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::SHR:
      sectionMemory.writeInstruction({OperationCodes::SHR, pars[1], pars[1], pars[0], 0});
      break;
    case InstructionTypes::CSRRD:
      sectionMemory.writeInstruction({OperationCodes::LD_CSR_REG, pars[1], pars[0], 0, 0});
      break;
    case InstructionTypes::CSRWR:
      sectionMemory.writeInstruction({OperationCodes::LD_REG_CSR, pars[1], pars[0], 0, 0});
      break;
    default:
      throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
      break;
  }

  locationCounter += WORD_SIZE;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertLoadInstructionRegister(MemoryInstructionType instructionType, const Parameters& parameters)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];

  uint32_t numInstructions = 0;
  switch(instructionType)
  {
    case MemoryInstructionType::REG_IMM:
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]);
      uint8_t destReg = std::get<uint8_t>(parameters[1]);
      sectionMemory.writeInstruction({OperationCodes::LD_REG_IMM, destReg, srcReg, 0, 0});
      
      numInstructions = 1;
      break;
    }
    case MemoryInstructionType::REG_MEM_DIR:
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]);
      uint8_t destReg = std::get<uint8_t>(parameters[1]);

      sectionMemory.writeInstruction({OperationCodes::LD_REG_MEM_DIR, destReg, srcReg, 0, 0});

      numInstructions = 1;
      break;
    }
    case MemoryInstructionType::REG_MEM_DIR_INC:
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]); // SP
      uint8_t destReg = std::get<uint8_t>(parameters[1]);
      uint8_t increment = std::get<uint8_t>(parameters[2]); // 4

      sectionMemory.writeInstruction({OperationCodes::LD_REG_MEM_DIR_INC, destReg, srcReg, 0, increment});

      numInstructions = 1;
      break;
    }
    case MemoryInstructionType::CSR_MEM_DIR_INC:
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]); // SP
      uint8_t destStatusReg = std::get<uint8_t>(parameters[1]); // status
      uint8_t increment = std::get<uint8_t>(parameters[2]); // 4

      sectionMemory.writeInstruction({OperationCodes::LD_CSR_MEM_DIR_INC, destStatusReg, srcReg, 0, increment});
      numInstructions = 1;
      break;
    }
    default:
      throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
      break;
  }

  locationCounter += WORD_SIZE * numInstructions;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertLoadInstructionLiteral(MemoryInstructionType instructionType, const Parameters& parameters)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  std::vector<LiteralPoolPatch>& poolPatches = sectionPoolPatchesMap[currentSectionNumber];

  uint32_t numInstructions = 0;
  switch(instructionType)
  {
    case MemoryInstructionType::LIT_IMM:
    {
      uint32_t srcLit = std::get<uint32_t>(parameters[0]);
      uint8_t destReg = std::get<uint8_t>(parameters[1]);

      uint32_t poolOffset = sectionMemory.writeLiteral(srcLit);
      AssemblerInstruction instruction {OperationCodes::LD_REG_MEM_DIR, destReg, PC, 0, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke 

      numInstructions = 1;
      break;
    }
    case MemoryInstructionType::LIT_MEM_DIR:
    {
      uint32_t srcLit = std::get<uint32_t>(parameters[0]);
      uint8_t destReg = std::get<uint8_t>(parameters[1]);

      uint32_t poolOffset = sectionMemory.writeLiteral(srcLit);
      AssemblerInstruction instruction {OperationCodes::LD_REG_MEM_DIR, destReg, PC, 0, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke
      
      // u sledecoj instrukciji cemo imati vrednost literala u registru i onda radimo load
      sectionMemory.writeInstruction({OperationCodes::LD_REG_MEM_DIR, destReg, destReg, 0, 0});

      numInstructions = 2;
      break;
    }
    case MemoryInstructionType::REG_REL_LIT:
    {
      uint8_t offsetReg = std::get<uint8_t>(parameters[0]);
      uint32_t offsetLit = std::get<uint32_t>(parameters[1]);
      uint8_t destReg = std::get<uint8_t>(parameters[2]);

      int offsetInt = static_cast<int>(offsetLit);
      if(std::abs(offsetInt) >= VALUE_OVERFLOW_LIMIT)
      {
        throw AssemblerError(ErrorCode::VALUE_OVERFLOW);
      }
      uint16_t offset = static_cast<uint16_t>(offsetLit); // ne treba cuvanje u bazenu jer je garantovano manje od 12B
      sectionMemory.writeInstruction({OperationCodes::LD_REG_MEM_DIR, destReg, offsetReg, 0, offset});

      numInstructions = 1;
      break;
    }
    default:
      throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
      break;
  }

  locationCounter += WORD_SIZE * numInstructions;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertLoadInstructionSymbol(MemoryInstructionType instructionType, const Parameters& parameters)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  std::string symbolName = std::get<std::string>(parameters[0]);
  uint8_t destReg = std::get<uint8_t>(parameters[1]); 

  uint32_t symbolIndex = findSymbol(symbolName);
  if(symbolIndex == INVALID) // ne nalazi se u tabeli simbola
  {
    symbolIndex = symbolTable.size();
    symbolTable.emplace_back(symbolName, INVALID, INVALID, false, false, false, UNUSED);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  std::vector<LiteralPoolPatch>& poolPatches = sectionPoolPatchesMap[currentSectionNumber];
  std::vector<SymbolUsage>& usages = symbolTable[symbolIndex].symbolUsages;

  // smestanje simbola u bazen literala ako vec nije tamo, ako jeste nadjemo gde se nalazi
  // kada smestimo simbol u bazen tretiramo ga kao literal
  uint32_t poolOffset = findPoolOffset(symbolIndex);
  if(poolOffset == UINT32_MAX)
  {
    poolOffset = sectionMemory.writeLiteral(0); // pravimo praznu rec koju cemo posle popuniti
    AssemblerInstruction instruction { OperationCodes::POOL, 0, 0, 0, 0 };
    usages.emplace_back(instruction, currentSectionNumber, poolOffset); // po oc cemo znati da je offset za pool
  }

  size_t numInstructions = 0;
  switch(instructionType)
  {
    case MemoryInstructionType::SYM_IMM:
    {
      AssemblerInstruction instruction {OperationCodes::LD_REG_MEM_DIR, destReg, PC, 0, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke 
      
      numInstructions = 1;
      break;
    }
    case MemoryInstructionType::SYM_MEM_DIR:
    {
      AssemblerInstruction instruction {OperationCodes::LD_REG_MEM_DIR, destReg, PC, 0, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke
      
      // u sledecoj instrukciji cemo imati vrednost simbola u registru i onda radimo load
      sectionMemory.writeInstruction({OperationCodes::LD_REG_MEM_DIR, destReg, destReg, 0, 0});

      numInstructions = 2;
      break;
    }
    default:
      throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
  }

  locationCounter += WORD_SIZE * numInstructions;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertStoreInstructionRegister(MemoryInstructionType instructionType, const Parameters&& parameters)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  std::vector<LiteralPoolPatch>& poolPatches = sectionPoolPatchesMap[currentSectionNumber];

  uint32_t numInstructions = 0;
  switch(instructionType)
  {
    case MemoryInstructionType::REG_IMM: // %reg(1) = %reg(0)
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]);
      uint8_t destReg = std::get<uint8_t>(parameters[1]);
      // koristimo load jer je isto kao load samo je destinacija obrnuta
      sectionMemory.writeInstruction({OperationCodes::LD_REG_IMM, destReg, srcReg, 0});
      
      numInstructions = 1;
      break;
    }
    case MemoryInstructionType::REG_MEM_DIR: // mem[%reg(1)] = reg[0]
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]);
      uint8_t destReg = std::get<uint8_t>(parameters[1]); // upisujemo na mem[reg]

      sectionMemory.writeInstruction({OperationCodes::ST_MEM_DIR, destReg, 0, srcReg, 0});

      numInstructions = 1;
      break;
    }
    case MemoryInstructionType::REG_MEM_DIR_INC: // SP(1) = SP(1) - 4, mem[SP(1)] = reg(0)
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]);
      uint8_t destReg = std::get<uint8_t>(parameters[1]); // SP
      uint8_t offset = std::get<uint8_t>(parameters[2]); // -4 

      sectionMemory.writeInstruction(
        {OperationCodes::ST_MEM_DIR_INC, destReg, 0, srcReg, offset});

      numInstructions = 1;
      break;
    }
    default:
        throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
  }

  locationCounter += WORD_SIZE * numInstructions;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertStoreInstructionLiteral(MemoryInstructionType instructionType, const Parameters&& parameters)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  std::vector<LiteralPoolPatch>& poolPatches = sectionPoolPatchesMap[currentSectionNumber];

  uint32_t numInstructions = 0;
  switch(instructionType)
  {
    case MemoryInstructionType::LIT_MEM_DIR: // mem[mem[PC + pomeraj(literal(1))]] = reg(0) <==> mem[literal] = reg
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]);
      uint32_t destLit = std::get<uint32_t>(parameters[1]);

      uint32_t poolOffset = sectionMemory.writeLiteral(destLit);
      AssemblerInstruction instruction {OperationCodes::ST_MEM_IND, PC, 0, srcReg, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

      numInstructions = 1;
      break;
    }
    case MemoryInstructionType::REG_REL_LIT: // mem[%reg(1) + literal(2)] = %reg(0)
    {
      uint8_t srcReg = std::get<uint8_t>(parameters[0]);
      uint8_t offsetReg = std::get<uint8_t>(parameters[1]);
      uint32_t offsetLit = std::get<uint32_t>(parameters[2]);

      int offsetInt = static_cast<int>(offsetLit);
      if(std::abs(offsetInt) >= VALUE_OVERFLOW_LIMIT)
      {
        throw AssemblerError(ErrorCode::VALUE_OVERFLOW);
      }
      uint16_t offset = static_cast<uint16_t>(offsetLit); // ne treba cuvanje u bazenu jer je garantovano manje od 12B
      sectionMemory.writeInstruction({OperationCodes::ST_MEM_DIR, offsetReg, 0, srcReg, offset});

      numInstructions = 1;
      break;
    }
    default:
      throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
      break;
  }

  locationCounter += WORD_SIZE * numInstructions;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertStoreInstructionSymbol(MemoryInstructionType instructionType, const Parameters& parameters)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  uint8_t srcReg = std::get<uint8_t>(parameters[0]); 
  std::string symbolName = std::get<std::string>(parameters[1]);

  uint32_t symbolIndex = findSymbol(symbolName);
  if(symbolIndex == INVALID) // ne nalazi se u tabeli simbola
  {
    symbolIndex = symbolTable.size();
    symbolTable.emplace_back(symbolName, INVALID, INVALID, false, false, false, UNUSED);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  std::vector<LiteralPoolPatch>& poolPatches = sectionPoolPatchesMap[currentSectionNumber];
  std::vector<SymbolUsage>& usages = symbolTable[symbolIndex].symbolUsages;

  // smestanje simbola u bazen literala ako vec nije tamo, ako jeste nadjemo gde se nalazi
  // kada smestimo simbol u bazen tretiramo ga kao literal
  uint32_t poolOffset = findPoolOffset(symbolIndex);
  if(poolOffset == UINT32_MAX)
  {
    poolOffset = sectionMemory.writeLiteral(0); // pravimo praznu rec koju cemo posle popuniti
    AssemblerInstruction instruction { OperationCodes::POOL, 0, 0, 0, 0 };
    usages.emplace_back(instruction, currentSectionNumber, poolOffset); // po oc cemo znati da je offset za pool
  }

  size_t numInstructions = 0;
  switch(instructionType)
  {
    case MemoryInstructionType::SYM_MEM_DIR:
    {
      AssemblerInstruction instruction {OperationCodes::ST_MEM_IND, PC, 0, srcReg, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

      numInstructions = 1;
      break;
    }
    default:
      throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
  }

  locationCounter += WORD_SIZE * numInstructions;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertJumpInstructionLiteral(InstructionTypes instructionType, const Parameters&& parameters)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  std::vector<LiteralPoolPatch>& poolPatches = sectionPoolPatchesMap[currentSectionNumber];
  uint32_t numInstructions = 0;
  
  switch (instructionType)
  {
  case InstructionTypes::CALL:
  {
    uint32_t litOperand = std::get<uint32_t>(parameters[0]);
    
    uint32_t poolOffset = sectionMemory.writeLiteral(litOperand);
    AssemblerInstruction instruction {OperationCodes::CALL_REG_IND, PC, 0, 0, 0};
    poolPatches.push_back({instruction, poolOffset, locationCounter});
    sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

    numInstructions = 1;
    break;
  }
  case InstructionTypes::JMP:
  {
    uint32_t litOperand = std::get<uint32_t>(parameters[0]);

    uint32_t poolOffset = sectionMemory.writeLiteral(litOperand);
    AssemblerInstruction instruction {OperationCodes::JMP_MEM_DIR, PC, 0, 0, 0};
    poolPatches.push_back({instruction, poolOffset, locationCounter});
    sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

    numInstructions = 1;
    break;
  }
  case InstructionTypes::BEQ:
  {
    uint8_t regB = std::get<uint8_t>(parameters[0]);
    uint8_t regC = std::get<uint8_t>(parameters[1]);
    uint32_t litOperand = std::get<uint32_t>(parameters[2]);

    uint32_t poolOffset = sectionMemory.writeLiteral(litOperand);
    AssemblerInstruction instruction {OperationCodes::BEQ_MEM_DIR, PC, regB, regC, 0};
    poolPatches.push_back({instruction, poolOffset, locationCounter});
    sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

    numInstructions = 1;
    break;
  }
  case InstructionTypes::BNE:
  {
    uint8_t regB = std::get<uint8_t>(parameters[0]);
    uint8_t regC = std::get<uint8_t>(parameters[1]);
    uint32_t litOperand = std::get<uint32_t>(parameters[2]);

    uint32_t poolOffset = sectionMemory.writeLiteral(litOperand);
    AssemblerInstruction instruction {OperationCodes::BNE_MEM_DIR, PC, regB, regC, 0};
    poolPatches.push_back({instruction, poolOffset, locationCounter});
    sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

    numInstructions = 1;
    break;
  }
  case InstructionTypes::BGT:
  {
    uint8_t regB = std::get<uint8_t>(parameters[0]);
    uint8_t regC = std::get<uint8_t>(parameters[1]);
    uint32_t litOperand = std::get<uint32_t>(parameters[2]);

    uint32_t poolOffset = sectionMemory.writeLiteral(litOperand);
    AssemblerInstruction instruction {OperationCodes::BGT_MEM_DIR, PC, regB, regC, 0};
    poolPatches.push_back({instruction, poolOffset, locationCounter});
    sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

    numInstructions = 1;
    break;
  }
  default:
    throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
  }

  locationCounter += WORD_SIZE * numInstructions;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::insertJumpInstructionSymbol(InstructionTypes instructionType, const Parameters&& parameters)
{
  if(currentSectionNumber == INVALID)
  {
    throw AssemblerError(ErrorCode::INSTRUCTION_OUTSIDE_OF_SECTION);
  }

  std::string symbolName;
  switch(instructionType)
  {
    case InstructionTypes::CALL:
    case InstructionTypes::JMP:
      symbolName = std::get<std::string>(parameters[0]);
      break;
    case InstructionTypes::BEQ:
    case InstructionTypes::BNE:
    case InstructionTypes::BGT:
      symbolName = std::get<std::string>(parameters[2]);
      break;
    default:
      throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
  }

  uint32_t symbolIndex = findSymbol(symbolName);
  if(symbolIndex == INVALID) // ne nalazi se u tabeli simbola
  {
    symbolIndex = symbolTable.size();
    symbolTable.emplace_back(symbolName, INVALID, INVALID, false, false, false, UNUSED);
  }

  SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  std::vector<LiteralPoolPatch>& poolPatches = sectionPoolPatchesMap[currentSectionNumber];
  std::vector<SymbolUsage>& usages = symbolTable[symbolIndex].symbolUsages;

  // smestanje simbola u bazen literala ako vec nije tamo, ako jeste nadjemo gde se nalazi
  // kada smestimo simbol u bazen tretiramo ga kao literal
  uint32_t poolOffset = findPoolOffset(symbolIndex);
  if(poolOffset == UINT32_MAX)
  {
    poolOffset = sectionMemory.writeLiteral(0); // pravimo praznu rec koju cemo posle popuniti
    AssemblerInstruction instruction { OperationCodes::POOL, 0, 0, 0, 0 };
    usages.emplace_back(instruction, currentSectionNumber, poolOffset); // po oc cemo znati da je offset za pool
  }

  size_t numInstructions = 0;
  switch(instructionType)
  {
    case InstructionTypes::CALL:
    {
      AssemblerInstruction instruction {OperationCodes::CALL_REG_IND, PC, 0, 0, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

      numInstructions = 1;
      break;
    }
    case InstructionTypes::JMP:
    {
      AssemblerInstruction instruction {OperationCodes::JMP_MEM_DIR, PC, 0, 0, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

      numInstructions = 1;
      break;
    }
    case InstructionTypes::BEQ:
    {
      uint8_t regB = std::get<uint8_t>(parameters[0]);
      uint8_t regC = std::get<uint8_t>(parameters[1]);

      AssemblerInstruction instruction {OperationCodes::BEQ_MEM_DIR, PC, regB, regC, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

      numInstructions = 1;
      break;
    }
    case InstructionTypes::BNE:
    {
      uint8_t regB = std::get<uint8_t>(parameters[0]);
      uint8_t regC = std::get<uint8_t>(parameters[1]);

      AssemblerInstruction instruction {OperationCodes::BNE_MEM_DIR, PC, regB, regC, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

      numInstructions = 1;
      break;
    }
    case InstructionTypes::BGT:
    {
      uint8_t regB = std::get<uint8_t>(parameters[0]);
      uint8_t regC = std::get<uint8_t>(parameters[1]);

      AssemblerInstruction instruction {OperationCodes::BGT_MEM_DIR, PC, regB, regC, 0};
      poolPatches.push_back({instruction, poolOffset, locationCounter});
      sectionMemory.writeBSS(4); // pravimo praznu instrukciju pa cemo je kasnije popuniti kad budemo imali podatke

      numInstructions = 1;
      break;
    }
    default:
      throw AssemblerError(ErrorCode::UNRECOGNIZED_INSTRUCTION);
  }

  locationCounter += WORD_SIZE * numInstructions;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::endAssembly()
{
  closeCurrentSection();

  validateSymbolTable();

  backpatch();
  patchFromLiteralPool();
  createRelocationTables();

  AssemblerOutputData data {symbolTable, sectionOrder, sectionMemoryMap, sectionRelocationMap};
  ObjectFileProcessor::writeToFile(data, outputFilePath);
  AssemblerTablesPrinter::printTables(data, outputFilePath);
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Assembler::findSymbol(const std::string& symbolName) const
{
  for(uint32_t i = 0, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    if(symbolTable[i].name == symbolName)
    {
      return i;
    }
  }

  return INVALID;
}
//-----------------------------------------------------------------------------------------------------------
uint32_t Assembler::findPoolOffset(uint32_t symbolIndex) const
{
  for(const SymbolUsage& usage : symbolTable[symbolIndex].symbolUsages)
  {
    if(usage.instruction.oc == OperationCodes::POOL)
    {
      return usage.sectionOffset;
    }
  }

  return UINT32_MAX;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::closeCurrentSection()
{
  if(currentSectionNumber == INVALID)
  {
    return;
  }
  
  // obrada stare sekcije
  const SectionMemory& sectionMemory = sectionMemoryMap[currentSectionNumber];
  symbolTable[currentSectionNumber].size = sectionMemory.getSectionSize();
  
  // resetovanje podataka
  locationCounter = 0;
  currentSectionNumber = INVALID;
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::validateSymbolTable()
{
  for(uint32_t i = 0, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    const Symbol& symbol = symbolTable[i];
    
    if(symbol.sectionNumber == i) // sekcija
    {
      continue;
    }

    if(!symbol.isDefined)
    {
      throw RuntimeError("AsemblerError: Simbol " + symbol.name + " koriscen bez da je definisan!");
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::patchFromLiteralPool()
{
  for(auto& [sectionNumber, poolPatches] : sectionPoolPatchesMap)
  {
    SectionMemory& sectionMemory = sectionMemoryMap[sectionNumber];

    for(LiteralPoolPatch& poolPatch : poolPatches)
    {
      AssemblerInstruction& instruction = poolPatch.instruction;
      
      // pomeraj od instrukcije do literala u bazenu
      // pretpostavka da literal uvek ide u pomeraj (trenutno za svaku instrukciju)
      instruction.disp = (sectionMemory.getCodeSize() + poolPatch.poolOffset) - poolPatch.sectionOffset;
      sectionMemory.repairMemory(poolPatch.sectionOffset, SectionMemory::toMemorySegment(instruction));
      break;
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
/*
Prolazi kroz sva koriscenja simbola u tabeli simbola i ako je:
  - lokalni simbol: upisuje njegovu vrednost
  - globalni simbol: upisuje 0 
*/ 
void Assembler::backpatch()
{
  for(int i = 1, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    const Symbol& symbol = symbolTable[i]; 
    if(symbol.sectionNumber == i) // simbol je sekcija
    {
      continue;
    }

    for(const SymbolUsage& usage : symbol.symbolUsages)
    {
      SectionMemory& sectionMemory = sectionMemoryMap[usage.sectionNumber];

      AssemblerInstruction instruction = usage.instruction;
      uint32_t value = symbol.isGlobal ? 0 : symbol.value;
      switch(instruction.oc)
      {
        case OperationCodes::WORD:
          sectionMemory.repairMemory(usage.sectionOffset, SectionMemory::toMemorySegment(value));
          break;
        case OperationCodes::POOL:
          sectionMemory.repairLiteralPool(usage.sectionOffset, SectionMemory::toMemorySegment(value));
          break;
        default:
          throw AssemblerError(ErrorCode::BACKPATCHING_ERROR);
      }
    }
  }
}
//-----------------------------------------------------------------------------------------------------------
void Assembler::createRelocationTables()
{
  for(int i = 1, tableSize = symbolTable.size(); i < tableSize; ++i)
  {
    const Symbol& symbol = symbolTable[i]; 
    if(symbol.sectionNumber == i) // simbol je sekcija
    {
      continue;
    }

    for(const SymbolUsage& usage : symbol.symbolUsages)
    {
      // za sekciju gde se koristi simbol pravimo referencu ka mestu gde je simbol definisan
      // Relokacioni zapis ce u sustini biti samo ka bazenu literala gde se simbol nalazi
      uint32_t symbolTableReference = (symbol.isGlobal || symbol.isExtern) ? i : symbol.sectionNumber;
      sectionRelocationMap[usage.sectionNumber]
        .emplace_back(usage.instruction.oc, usage.sectionOffset, symbolTableReference);
    }
  }
}

} // namespace asm_core