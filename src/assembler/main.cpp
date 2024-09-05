#include <inc/assembler/assembler.hpp>

#include <memory>

namespace Assembler
{
  std::unique_ptr<Assembler> assembler;
  uint32_t currentSourceFileLine = 0;

  int main(int argc, char* argv[])
  {
    assembler = std::make_unique<Assembler>();
  }
}

