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

    static Symbol parseSymbol(const std::string& line) {
        std::istringstream lineStream(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(lineStream, token, ':')) {
            tokens.push_back(token);
        }

        return Symbol(tokens[0], std::stoi(tokens[1]), std::stoi(tokens[2]), 
                      tokens[3] == "1", tokens[4] == "1", tokens[5] == "1", std::stoi(tokens[6]));
    }

    static uint32_t parseSectionId(const std::string& line) {
        std::istringstream lineStream(line);
        std::string token;
        std::getline(lineStream, token, ' ');
        return std::stoi(token);
    }

    static RelocationEntry parseRelocationEntry(const std::string& line) {
        std::istringstream lineStream(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(lineStream, token, ':')) {
            tokens.push_back(token);
        }

        // Pretpostavljam da AssemblerInstruction ima neku osnovnu konverziju iz stringa
        AssemblerInstruction instruction = parseInstruction(tokens[0]);
        return RelocationEntry(instruction, std::stoi(tokens[1]), std::stoi(tokens[2]));
    }

    static void parseSectionData(const std::string& line, std::vector<uint8_t>& sectionMemory) {
        std::istringstream lineStream(line);
        std::string token;
        std::vector<uint8_t> sectionData;

        while (lineStream >> std::hex >> token)
        {
            sectionMemory.push_back(static_cast<uint8_t>(std::stoi(token, nullptr, 16)));
        }
    }

    static AssemblerInstruction parseInstruction(const std::string& instructionString) {
        // Parsiranje instrukcije, ovo je samo placeholder
        return AssemblerInstruction();
    }
};


} // common