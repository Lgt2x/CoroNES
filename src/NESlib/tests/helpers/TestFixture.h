#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Bus.h"
#include "CPU.h"
#include "Assembler.h"
#include "mappers/DummyMapper.h"

namespace TestFixture {

struct NES_Test {
  std::unique_ptr<CPU_6502> cpu;
  std::unique_ptr<Bus> bus;
};

/**
 * Create a CPU and Bus object with a given program,
 * located at address 0x800.
 */
inline NES_Test setupTest(std::vector<std::string> program, uint16_t startAddress=0x800) {
  auto mapper = std::make_unique<DummyMapper>();
  auto bus = std::make_unique<Bus>(mapper.get());

  // Set reset vector
  bus->writeByte(0xFFFC, startAddress & 0xFF);
  bus->writeByte(0xFFFD, startAddress >> 8);

  // Assemble code and write compiled binary to memory
  auto compiled = Assembler().assemble(program);
  int PC = 0x800;
  for (auto programByte : compiled) {
    bus->writeByte(PC++, programByte);
  }

  // Setup the CPU
  auto cpu = std::make_unique<CPU_6502>(bus.get());
  cpu->reset();

  return NES_Test{std::move(cpu), std::move(bus)};
}

inline NES_Test setupTestAndExecute(std::vector<std::string> program, uint16_t startAddress=0x800) {
  NES_Test fixture = setupTest(program, startAddress);
  fixture.cpu->step(program.size());
  return std::move(fixture);
}

}