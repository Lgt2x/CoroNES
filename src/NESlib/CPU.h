#pragma once

#include <cstdint>
#include "Bus.h"



class CPU_6502 {
public:
  explicit CPU_6502(Bus* ram);

  // Execute one instruction, pointed by the Program Counter
  void step();

  // Hard reset
  void reset();

  // Debug functions
  void print_state() const;

  std::vector<uint8_t> dumpRegisters();

  template<typename T>
  static std::string print_hex(T a);


private:
  // 8-bit general purpose registers

  //TODO : struct for CPU registers, bitset for flags ?
  uint8_t A;
  uint8_t X;
  uint8_t Y;

  uint16_t PC;                  // Program counter
  std::vector<bool> flags;      // Status register, NVBDIZC
  uint8_t SP;                   // Stack pointer

  // Interruption vectors
  uint16_t reset_vector;

  Bus* ram;


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
   * for opcodes whose last 2 bits are 0b01, get the value depending on the addressing mode
   * and operand(s) if any.
   */
  uint8_t getMem_c1(uint8_t mode);
};