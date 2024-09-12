#pragma once

#include <common/assembler_common_structures.hpp>
#include <unordered_map>

using namespace common;

namespace lnk_core
{
  struct LinkerInputData
  {
    std::vector<Symbol> symbolTable;
    std::unordered_map<uint32_t, std::vector<uint8_t>> sectionMemoryMap;
    std::unordered_map<uint32_t, std::vector<RelocationEntry>> sectionRelocationMap;
  };
} // namespace lnk_core