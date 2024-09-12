#include <common/object_file_processor.hpp>
#include <common/exceptions.hpp>

#include <cstdint>

namespace
{
constexpr uint32_t INVALID_SECTION = 0;

uint32_t findSection(const AssemblerOutputData& data, const std::string& symbolName)
{
  for(uint32_t i = 0, tableSize = data.symbolTable.size(); i < tableSize; ++i)
  {
    if(data.symbolTable[i].name == symbolName)
    {
      return i;
    }
  }

  return INVALID_SECTION;
}

} // namespace unnamed

namespace common
{

enum class ReadMode
{
	SYMBOL_TABLE,
	RELOCATION_TABLE,
	GENERATED_CODE
};

void ObjectFileProcessor::writeToFile(const AssemblerOutputData& data, const std::string& filePath)
{
	std::ofstream outFile(filePath);
	if (!outFile.is_open()) 
	{
		throw RuntimeError("Couldn't open output file " + filePath + " in writing mode!");
	}

	// Tabela simbola
	outFile << "Sym:\n";
	for (const Symbol& symbol : data.symbolTable)
	{      
	outFile << symbol.name << ":" << symbol.sectionNumber << ":" << symbol.value << ":"
			<< (symbol.isGlobal ? "1" : "0") << ":" << (symbol.isExtern ? "1" : "0") << ":"
			<< (symbol.isDefined ? "1" : "0") << ":" << symbol.size << "\n";
	}
	
	// Relokacioni zapisi
	for (const std::string& sectionName : data.sectionOrder)
	{
		uint32_t sectionNumber = findSection(data, sectionName);
		if(data.sectionRelocationMap.find(sectionNumber) == data.sectionRelocationMap.end())
		{
			continue;
		}
		const auto& relocationEntries = data.sectionRelocationMap.at(sectionNumber);

		outFile << "Rel:" << sectionNumber << "\n";
		for (const RelocationEntry& entry : relocationEntries)
		{
			outFile << static_cast<int>(entry.operationCode)
							<< ":" << entry.offset
							<< ":" << entry.symbolTableReference << "\n";
		}
	}

	// Generisani kod
	for (const auto& sectionName : data.sectionOrder)
	{
		uint32_t sectionNumber = findSection(data, sectionName);
		if(data.sectionMemoryMap.find(sectionNumber) == data.sectionMemoryMap.end())
		{
			continue;
		}
		const auto& sectionMemory = data.sectionMemoryMap.at(sectionNumber);

		outFile << "Code:" << sectionNumber << "\n";
		for (uint8_t byte : sectionMemory.getSectionMemory())
		{
			outFile << (int)byte << " ";
		}
		outFile << "\n";
	}

	outFile.close();

}
//-----------------------------------------------------------------------------------------------------------
lnk_core::LinkerInputData ObjectFileProcessor::readFromFile(const std::string& filePath)
{
	lnk_core::LinkerInputData data;

	std::ifstream inFile(filePath);

	if (!inFile.is_open())
	{
		throw RuntimeError("Couldn't open output file " + filePath + " in writing mode!");
	}
	
	std::string line;
	ReadMode readMode;
	uint32_t sectionNumber = INVALID_SECTION;
	while (std::getline(inFile, line))
	{
		std::istringstream lineStream(line);
		std::string token;

		if (line.find("Sym:") != std::string::npos)
		{
			readMode = ReadMode::SYMBOL_TABLE;
			continue;
		}
		else if (line.find("Rel:") != std::string::npos)
		{
			readMode = ReadMode::RELOCATION_TABLE;
			sectionNumber = parseSectionNumber(line);
			std::vector<RelocationEntry> relocations;
			continue;
		}
		else if (line.find("Code:") != std::string::npos)
		{
			readMode = ReadMode::GENERATED_CODE;
			sectionNumber = parseSectionNumber(line);
			continue;
		}

		switch(readMode)
		{
			case ReadMode::SYMBOL_TABLE:
				data.symbolTable.emplace_back(parseSymbol(line));
				break;
			case ReadMode::RELOCATION_TABLE:
				if(sectionNumber == INVALID_SECTION)
				{
					throw LinkerError("Greska u parsiranj relokacione tabele!");
				}

				data.sectionRelocationMap[sectionNumber].emplace_back(parseRelocationEntry(line));
				break;
			case ReadMode::GENERATED_CODE:
				data.sectionMemoryMap[sectionNumber] = parseSectionData(line);
		}
	}

	inFile.close();
	return data;
}
//-----------------------------------------------------------------------------------------------------------
common::Symbol ObjectFileProcessor::parseSymbol(const std::string& line) {
	
	std::istringstream lineStream(line);
	std::string token;
	std::vector<std::string> tokens;

	while (std::getline(lineStream, token, ':'))
	{
		tokens.push_back(token);
	}
	
	return Symbol(tokens[0], std::stoul(tokens[1]), std::stoi(tokens[2]), 
								tokens[3] == "1", tokens[4] == "1", tokens[5] == "1", std::stoul(tokens[6]));
}
//-----------------------------------------------------------------------------------------------------------
uint32_t ObjectFileProcessor::parseSectionNumber(const std::string& line)
{ 
	uint32_t separatorPos = line.find(':');
	return std::stoul(line.substr(separatorPos + 1));
}
//-----------------------------------------------------------------------------------------------------------
RelocationEntry ObjectFileProcessor::parseRelocationEntry(const std::string& line)
{
	std::istringstream lineStream(line);
	std::string token;
	std::vector<std::string> tokens;

	while (std::getline(lineStream, token, ':'))
	{
			tokens.push_back(token);
	}

	OperationCodes oc = static_cast<OperationCodes>(std::stoi(tokens[0]));
	return RelocationEntry(oc, std::stoul(tokens[1]), std::stoul(tokens[2]));
}
//-----------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ObjectFileProcessor::parseSectionData(const std::string& line)
{
	std::istringstream lineStream(line);
	std::string token;

	std::vector<uint8_t> sectionData;

	while (lineStream >> token)
	{
			sectionData.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
	}

	return sectionData;
}

} // common