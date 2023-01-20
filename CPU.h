#pragma once

#include <cstdint>
#include "Parser.h"
#include "Bus.h"

class CPU_6502 {
private:
  // Registers
  char A;
  char X;
  char Y;

  char PC;      // Program counter
  char flags;   // NVBDIZC
  uint16_t SP;     // Stack pointer

  std::unique_ptr<Bus>& ram;
  // Map to associate the strings with the enum values
  uint16_t getRAMAddress(int mode, uint16_t address);
  char getValue(int mode, uint16_t address);
  void setRAMValue(uint16_t mode, uint16_t address, char byte);

public:
  explicit CPU_6502(std::unique_ptr<Bus>& ram);

  int execute(const Instruction& i);

  void print_reg() const;
  template<typename T>static std::string print_hex(T a);
};