#pragma once

#include <cstdint>
#include "Parser.h"
#include "RAM.h"

enum Codes {
  ADC,
  AND,
  ASL,
  BBC,
  BCS,
  BEQ,
  BIT,
  BMI,
  BNE,
  BPL,
  BRK,
  BVC,
  BVS,
  CLC,
  CLD,
  CLI,
  CLV,
  CMP,
  CPX,
  CPY,
  DEC,
  DEX,
  DEY,
  EOR,
  INC,
  INX,
  INY,
  JMP,
  JSR,
  LDA,
  LDX,
  LDY,
  LSR,
  NOP,
  ORA,
  PHA,
  PHP,
  PLA,
  PLP,
  ROL,
  ROR,
  RTI,
  RTS,
  SBC,
  SEC,
  SED,
  SEI,
  STA,
  STX,
  STY,
  TAX,
  TAY,
  TSX,
  TXA,
  TXS,
  TYA,
};

class CPU_6502 {
private:
  // Registers
  char A;
  char X;
  char Y;

  char PC;      // Program counter
  char flags;   // NVBDIZC
  uint16_t SP;     // Stack pointer

  std::unique_ptr<RAM>& ram;
  // Map to associate the strings with the enum values
  std::map<std::string, Codes> mapCodes;
  void init_codes();
  uint16_t getRAMAddress(int mode, uint16_t address);
  char getValue(int mode, uint16_t address);
  void setRAMValue(uint16_t mode, uint16_t address, char byte);

  // Instructions
  void i_ADC(const Instruction& i);
  void i_AND(const Instruction& i);
  void i_ASL(const Instruction& i);
  void i_BBC(const Instruction& i);
  void i_BCS(const Instruction& i);
  void i_BEQ(const Instruction& i);
  void i_BIT(const Instruction& i);
  void i_BMI(const Instruction& i);
  void i_BNE(const Instruction& i);
  void i_BPL(const Instruction& i);
  void i_BRK(const Instruction& i);
  void i_BVC(const Instruction& i);
  void i_BVS(const Instruction& i);
  void i_CLC(const Instruction& i);
  void i_CLD(const Instruction& i);
  void i_CLI(const Instruction& i);
  void i_CLV(const Instruction& i);
  void i_CMP(const Instruction& i);
  void i_CPX(const Instruction& i);
  void i_CPY(const Instruction& i);
  void i_DEC(const Instruction& i);
  void i_DEX(const Instruction& i);
  void i_DEY(const Instruction& i);
  void i_EOR(const Instruction& i);
  void i_INC(const Instruction& i);
  void i_INX(const Instruction& i);
  void i_INY(const Instruction& i);
  void i_JMP(const Instruction& i);
  void i_JSR(const Instruction& i);
  void i_LDA(const Instruction& i);
  void i_LDX(const Instruction& i);
  void i_LDY(const Instruction& i);
  void i_LSR(const Instruction& i);
  void i_NOP(const Instruction& i) {};
  void i_ORA(const Instruction& i);
  void i_PHA(const Instruction& i);
  void i_PHP(const Instruction& i);
  void i_PLA(const Instruction& i);
  void i_PLP(const Instruction& i);
  void i_ROL(const Instruction& i);
  void i_ROR(const Instruction& i);
  void i_RTI(const Instruction& i);
  void i_RTS(const Instruction& i);
  void i_SBC(const Instruction& i);
  void i_SEC(const Instruction& i);
  void i_SED(const Instruction& i);
  void i_SEI(const Instruction& i);
  void i_STA(const Instruction& i);
  void i_STX(const Instruction& i);
  void i_STY(const Instruction& i);
  void i_TAX(const Instruction& i);
  void i_TAY(const Instruction& i);
  void i_TSX(const Instruction& i);
  void i_TXA(const Instruction& i);
  void i_TXS(const Instruction& i);
  void i_TYA(const Instruction& i);
public:
  explicit CPU_6502(std::unique_ptr<RAM>& ram);

  int execute(const Instruction& i);

  void print_reg() const;
  template<typename T>static std::string print_hex(T a);
};