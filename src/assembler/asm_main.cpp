#include <assembler/assembler.hpp>
#include <assembler/assembler_common.hpp>
#include <common/exceptions.hpp>

#include <cstring>
#include <iostream>
#include <string>

using namespace common;

#define YYERROR_VERBOSE

extern void yyerror(const char*);
extern FILE *yyin;
extern int yyparse();

int main(int argc, char* argv[])
{
  std::string inputFilePath, outputFilePath;

	try
	{
		if(argc != 4 || strcmp(argv[1], "-o") != 0) // program -o izlaz ulaz
		{
			throw RuntimeError("Greska! Ispravna Sintaksa: ./assembler -o izlaz.o ulaz.s");
		}

		outputFilePath = argv[2];
		inputFilePath = argv[3];

		AssemblerCommon::assembler = new asm_core::Assembler(outputFilePath);

		FILE* inputFile = fopen(inputFilePath.c_str(), "rw+");
		if(inputFile == nullptr)
		{
			throw RuntimeError("Greska pri otvaranju ulaznog fajla " + inputFilePath + "!");
		}

		yyin = inputFile;
		yyparse();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return -1;
	}
	
}