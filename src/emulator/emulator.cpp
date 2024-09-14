#include <emulator/emulator.hpp>

#include <common/executable_file_processor.hpp>

using namespace common;

namespace
{
constexpr uint64_t DEFAULT_MEMORY_SIZE = (1ULL << 32);
} // unnamed

namespace emulator_core
{

Emulator::Emulator(const std::string& inputFilePath)
  : inputFilePath(inputFilePath), memory(DEFAULT_MEMORY_SIZE) {}

void Emulator::emulate()
{
  const auto& segments = ExecutableFileProcessor::readFromFile(inputFilePath);
  memory.initMemory(segments);
}

} // namespace emulator_core