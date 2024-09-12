#include <common/object_file_processor.hpp>
#include <common/exceptions.hpp>

namespace common
{

void ObjectFileProcessor::writeToFile(const AssemblerOutputData& data, const std::string& filePath)
{
	std::ofstream outFile(filePath);
	if (!outFile.is_open()) 
	{
		throw RuntimeError("Couldn't open output file " + filePath + " in writing mode!");
	}

	// Tabela simbola
	outFile << "Symbol Table:\n";
	for (const Symbol& symbol : data.symbolTable)
	{      
	outFile << symbol.name << ":" << symbol.sectionNumber << ":" << symbol.value << ":"
			<< (symbol.isGlobal ? "1" : "0") << ":" << (symbol.isExtern ? "1" : "0") << ":"
			<< (symbol.isDefined ? "1" : "0") << ":" << symbol.size << "\n";
	}

	// Relokacioni zapisi
	for (const auto& [sectionId, relocationEntries] : data.sectionRelocationMap)
	{
		outFile << "Section " << sectionId << " Relocation Entries:\n";
		for (const RelocationEntry& entry : relocationEntries)
		{
			outFile << entry.offset << ":" << entry.symbolTableReference << "\n";
		}
	}

	// Generisani kod
	for (const auto& [sectionId, sectionMemory] : data.sectionMemoryMap)
	{
		outFile << "Section " << sectionId << " Code:\n";
		for (uint8_t byte : sectionMemory.getSectionMemory())
		{
			outFile << std::hex << (int)byte << " ";
		}
		outFile << "\n";
	}

	outFile.close();

}

lnk_core::LinkerInputData ObjectFileProcessor::readFromFile(const std::string& filePath)
{
	lnk_core::LinkerInputData data;

	std::ifstream inFile(filePath);
	if (!inFile.is_open())
	{
		throw RuntimeError("Couldn't open output file " + filePath + " in writing mode!");
	}

	std::string line;
	while (std::getline(inFile, line))
	{
		std::istringstream lineStream(line);
		std::string token;

		if (line.find("Symbol Table:") != std::string::npos)
		{
			while (std::getline(inFile, line) && !line.empty())
			{
				Symbol symbol = parseSymbol(line);
				data.symbolTable.push_back(symbol);
			}
		}
		else if (line.find("Relocation Entries:") != std::string::npos)
		{
			uint32_t sectionId = parseSectionId(line);
			std::vector<RelocationEntry> relocations;
			while (std::getline(inFile, line) && !line.empty())
			{
				RelocationEntry entry = parseRelocationEntry(line);
				relocations.push_back(entry);
			}
			data.sectionRelocationMap[sectionId] = relocations;
		}
		else if (line.find("Code:") != std::string::npos)
		{
			uint32_t sectionId = parseSectionId(line);
			std::vector<uint8_t> sectionMemory;
			while (std::getline(inFile, line) && !line.empty())
			{
				parseSectionData(line, sectionMemory);
			}
			data.sectionMemoryMap[sectionId] = sectionMemory;
		}
	}

	inFile.close();
	return data;
}

} // common