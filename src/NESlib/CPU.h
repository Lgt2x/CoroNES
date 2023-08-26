#pragma once

#include "Bus.h"
#include <bitset>
#include <cstdint>

class CPU_6502 {
private:
  Bus *ram;
  struct Registers {
    // 8-bit general purpose registers
    uint8_t A{};
    uint8_t X{};
    uint8_t Y{};

    uint16_t PC = 0x4000;       // Program counter
    std::bitset<8> flags{0x34}; // Status register, NVBDIZC
    uint8_t SP = 0xFD;          // Stack pointer
  } reg;

  uint16_t nmi_vector{};
  uint16_t reset_vector{};
  uint16_t irq_vector{};

  uint16_t readAddressAndIncrementPC(uint8_t mode);
  uint8_t readByteAndIncrementPC(uint8_t mode);
  uint8_t readByte(uint8_t mode);
  void writeByte(uint8_t mode, uint8_t value);

public:
  explicit CPU_6502(Bus *ram);

  void step();
  void step(int nbSteps);

  void reset();

  void printState() const;
  Registers dumpRegisters() { return reg; };

  template <typename T> static std::string print_hex(T a);
};
