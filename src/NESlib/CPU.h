#pragma once

#include "Bus.h"
#include <bitset>
#include <cstdint>

struct Registers {
  // 8-bit general purpose registers
  uint8_t A{};
  uint8_t X{};
  uint8_t Y{};

  uint16_t PC = 0x4000;       // Program counter
  std::bitset<8> flags{0x34}; // Status register, NVBDIZC
  uint8_t SP = 0xFD;          // Stack pointer
};

class CPU_6502 {
private:
  Registers reg;

  // Interruption vectors
  uint16_t reset_vector{};

  Bus *ram;

  /**
   * set flags corresponding to the provided mask,
   * if the value has the desired properties.
   */
  void setFlags(uint8_t mask, uint8_t value);

  /**
   * get flags as a single byte
   */
  [[nodiscard]] uint8_t flagsToByte() const;

  /**
   * for opcodes whose last 2 bits are 0b01, get the value depending on the
   * addressing mode and operand(s) if any.
   */
  uint16_t getAddress_c1(uint8_t mode);
  uint8_t getMem_c1(uint8_t mode);
  void writeMem_c1(uint8_t mode, uint8_t value);

public:
  explicit CPU_6502(Bus *ram);

  void step();
  void step(int nbSteps);

  void reset();

  void print_state() const;
  Registers dumpRegisters() { return reg; };

  template <typename T> static std::string print_hex(T a);
};
