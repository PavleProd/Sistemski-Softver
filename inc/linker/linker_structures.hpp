#pragma once

#include <common/assembler_common_structures.hpp>

#include <vector>
#include <limits>
#include <unordered_map>

namespace lnk_core
{

constexpr uint32_t INVALID_NUMBER = UINT32_MAX;

struct GlobalSectionData
{
  common::SectionMemory generatedCode;
  uint32_t startAddress = INVALID_NUMBER;
  uint32_t size = INVALID_NUMBER;
};

struct LinkerInputData
{
  std::vector<common::Symbol> symbolTable;
  std::unordered_map<uint32_t, std::vector<uint8_t>> sectionMemoryMap;
  std::unordered_map<uint32_t, std::vector<common::RelocationEntry>> sectionRelocationMap;
};

struct SectionPlacement
{
  std::string sectionName;
  uint32_t startAddress;
};

} // namespace lnk_core