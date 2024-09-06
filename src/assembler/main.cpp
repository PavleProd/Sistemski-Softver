#include <inc/assembler/assembler_common.hpp>

using namespace asm_core;

constexpr const char* outputFile = "asembler.o";

int main(int argc, char* argv[])
{
  assembler = std::make_unique<Assembler>();
}