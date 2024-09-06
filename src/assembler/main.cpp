#include <common/assembler_common.hpp>

using namespace common;

constexpr const char* outputFile = "asembler.o";

int main(int argc, char* argv[])
{
  AssemblerCommon::assembler = std::make_unique<asm_core::Assembler>();
}