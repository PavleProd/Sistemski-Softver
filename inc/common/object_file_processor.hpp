#pragma once

#include <common/assembler_common_structures.hpp>
#include <linker/linker_structures.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

namespace common
{

class ObjectFileProcessor
{
public:
    static void writeToFile(const AssemblerOutputData& data, const std::string& filePath);
    static lnk_core::LinkerInputData readFromFile(const std::string& filePath);
private:
    // Pomoćne funkcije za parsiranje

    static Symbol parseSymbol(const std::string& line);
    static uint32_t parseSectionNumber(const std::string& line);
    static RelocationEntry parseRelocationEntry(const std::string& line);
    static std::vector<uint8_t> parseSectionData(const std::string& line);
};


} // common