#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sys/types.h>
#include <unistd.h>

#include "Bus.h"
#include "CPU.h"

enum opcode_c0_b0_implied { BRK, JSR, RTI, RTS };
enum opcode_c0_b2_implied { PHP, PLP, PHA, PLA, DEY, TAY, INY, INX };
enum opcode_c0_b6_implied { CLC, SEC, CLI, SEI, TYA, CLV, CLD, SED };
enum opcode_c0 { BIT = 1, JMP_abs, JMP_ind, STY, LDY, CPY, CPX };
enum opcode_c0_branch { BPL, BMI, BVC, BVS, BCC, BCS, BNE, BEQ };
enum opcode_c2 { ASL, ROL, LSR, ROR, STX, LDX, DEC, INC };
enum adressingModes_c1 {
  X_IND,
  ZPG,
  IMM,
  ABS,
  IND_Y,
  ZPG_X,
  ABS_Y,
  ABS_X,
  ZPG_Y
};
enum adressingModes_c2 {
  IMMEDIATE,
  ZERO_PAGE,
  IMPLIED,
  ABSOLUTE,
  ZERO_PAGE_IDX = 5,
  STACK,
  ABSOLUTE_IDX
};
enum opcode_c1 { ORA, AND, EOR, ADC, STA, LDA, CMP, SBC };
enum flags { N_f, V_f, B_f, D_f, I_f, Z_f, C_f };

/******* Public functions *******/
CPU_6502::CPU_6502(Bus *ram) : ram(ram) { CPU_6502::reset(); }

void CPU_6502::reset() {
  reset_vector = ram->readByte(0xFFFC) | (ram->readByte(0xFFFD) << 8);
  reg.PC = reset_vector;
  reg.flags = std::bitset<8>{0b00110100};
}

void CPU_6502::step() {
  // CPU_6502::print_state();

  // Read the opcode at the current program counter address and increment it
  uint8_t opcode = ram->readByte(reg.PC);
  reg.PC++;

  /* Decompose the opcode as the binary vector 'aaabbbcc'
  In a general sense, a depends on the instruction, mode on the addressing mode,
   and c classifies opcodes with similar addressing properties.
   For legal instructions, c is never equal to 0b11.
   See https://www.masswerk.at/6502/6502_instruction_set.html
   for the full explanation and tables
   */
  uint8_t instruction = opcode >> 5;  // 0b11100000
  uint8_t mode = opcode >> 2 & 0b111; // 0b00011100
  uint8_t category = opcode & 0b11;   // 0b00000011

  uint8_t value = 0x0;

  switch (category) {
  case 0b00:
    std::cout << "Not supported yet." << std::endl;
    break;
  case 0b01:
    switch (instruction) {
    case ORA: // OR memory with accumulator
      reg.A |= this->getMemAndIncrementPC(mode);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    case AND: // AND memory with accumulator
      reg.A &= this->getMemAndIncrementPC(mode);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    case EOR: // XOR memory with accumulator
      reg.A ^= this->getMemAndIncrementPC(mode);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    case ADC: { // Add memory to accumulator with carry
      uint8_t operand = getMemAndIncrementPC(mode);
      uint16_t res = reg.A + operand;
      reg.A = res & 0xFF;
      reg.flags[C_f] = (res >> 8) > 0; // Set the carry
      // Set the overflow (V_f) flag. Is it set when the result changed the sign
      // bit when it should not have. e.g. when both input numbers have the sign
      // bit off but the result has the sign bit on. There is probably a more
      // efficient way to do this. See http://www.6502.org/tutorials/vflag.html
      // for full explanation
      reg.flags[V_f] = (reg.A & 0x80 && operand & 0x80 && !(res & 0x80)) ||
                       (!(reg.A & 0x80) && operand & 0x80 && res & 0x80);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    }
    case STA: // Store accumulator to memory
      this->writeMem_c1(mode, reg.A);
      break;
    case LDA: // Load accumulator with memory
      reg.A = this->getMemAndIncrementPC(mode);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    case CMP: { // Compare memory with accumulator
      uint8_t operand = getMemAndIncrementPC(mode);
      reg.flags[C_f] = reg.A >= operand;
      reg.flags[N_f] = reg.A & 0x80; // Not sure about that
      reg.flags[Z_f] = reg.A == operand;
      break;
    }
    case SBC: { // Subtract from accumulator with borrow
      uint8_t operand = getMemAndIncrementPC(mode);
      uint16_t res = reg.A - operand; // - reg.flags[C_f];
      reg.flags[C_f] = (res >> 8) > 0;
      reg.flags[V_f] = (reg.A & 0x80 && operand & 0x80 && !(res & 0x80)) ||
                       (!(reg.A & 0x80) && operand & 0x80 && res & 0x80);
      reg.A = res & 0xFF;
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    }
    }
    break;
  case 0b10:
    switch (instruction) {
    case ASL: {
      uint16_t value{};
      if (mode == IMM) {
        value = (reg.A << 1);
        reg.A = value & 0xFF;
      } else {
        uint16_t PC_save = reg.PC;
        value = (getMemAndIncrementPC(mode) << 1);
        reg.PC = PC_save;
        writeMem_c1(mode, value & 0xFF);
      }
      reg.flags[N_f] = value & 0x80;
      reg.flags[Z_f] = value == 0;
      reg.flags[C_f] = value > 0xFF;
      break;
    }
    case ROL: {
      uint16_t value{};
      if (mode == IMM) {
        value = (reg.A << 1) + reg.flags[C_f];
        reg.A = value & 0xFF;
      } else {
        uint16_t PC_save = reg.PC;
        value = (getMemAndIncrementPC(mode) << 1) + reg.flags[C_f];
        reg.PC = PC_save;
        writeMem_c1(mode, value & 0xFF);
      }
      reg.flags[N_f] = value & 0x80;
      reg.flags[Z_f] = value == 0x00;
      reg.flags[C_f] = value > 0xFF;
      break;
    }
    case LSR: {
      uint8_t value{};
      uint8_t previous{};
      if (mode == IMM) {
        previous = reg.A;
        value = (reg.A >> 1);
        reg.A = value;
      } else {
        uint16_t PC_save = reg.PC;
        previous = getMemAndIncrementPC(mode);
        value = (previous >> 1);
        reg.PC = PC_save;
        writeMem_c1(mode, value);
      }
      reg.flags[N_f] = value & 0x80;
      reg.flags[Z_f] = value == 0x00;
      reg.flags[C_f] = previous & 0x01;
      break;
    }
    case ROR: {
      uint8_t value{};
      uint8_t previous{};
      if (mode == IMM) {
        previous = reg.A;
        value = (reg.A >> 1) + (reg.flags[C_f] << 7);
        reg.A = value;
      } else {
        uint16_t PC_save = reg.PC;
        previous = getMemAndIncrementPC(mode);
        value = (previous >> 1);
        reg.PC = PC_save;
        writeMem_c1(mode, value);
      }
      reg.flags[N_f] = value & 0x80;
      reg.flags[Z_f] = value == 0x00;
      reg.flags[C_f] = previous & 0x01;
      break;
    }
    case STX: {
      switch (mode) {
      case ZERO_PAGE:
        writeMem_c1(ZPG, reg.X);
        break;
      case IMPLIED:
        reg.A = reg.X;
        break;
      case ABSOLUTE:
        writeMem_c1(ABS, reg.X);
        break;
      case ZERO_PAGE_IDX:
        writeMem_c1(ZPG_Y, reg.X);
        break;
      case STACK:
        reg.SP = reg.X;
        break;
      }
      break;
    }
    case LDX: {
      switch (mode) {
      case IMMEDIATE:
        reg.X = getMemAndIncrementPC(IMM);
        break;
      case ZERO_PAGE:
        reg.X = getMemAndIncrementPC(ZPG);
        break;
      case IMPLIED:
        reg.X = reg.A;
        break;
      case ABSOLUTE:
        reg.X = getMemAndIncrementPC(ABS);
        break;
      case ZERO_PAGE_IDX:
        reg.X = getMemAndIncrementPC(ZPG_Y);
        break;
      case STACK:
        reg.X = reg.SP;
        break;
      case ABSOLUTE_IDX:
        reg.X = getMemAndIncrementPC(ABS_Y);
        break;
      }
      reg.flags[N_f] = reg.X & 0x80;
      reg.flags[Z_f] = reg.X == 0x00;
      break;
    }
    case DEC: {
      uint8_t value{};
      switch (mode) {
      case ZERO_PAGE:
        value = getMem(ZPG) - 1;
        writeMem_c1(ZPG, value);
        break;
      case IMPLIED:
        value = --reg.X;
        break;
      case ABSOLUTE:
        value = getMem(ABS) - 1;
        writeMem_c1(ABS, value);
        break;
      case ZERO_PAGE_IDX:
        value = getMem(ZPG_X) - 1;
        writeMem_c1(ZPG_X, value);
        break;
      case ABSOLUTE_IDX:
        value = getMem(ABS_X) - 1;
        writeMem_c1(ABS_X, value);
        break;
      }
      reg.flags[N_f] = value & 0x80;
      reg.flags[Z_f] = value == 0x00;
      break;
    }
    case INC: {
      uint8_t value{};
      switch (mode) {
      case ZERO_PAGE:
        value = getMem(ZPG) + 1;
        writeMem_c1(ZPG, value);
        break;
      case IMPLIED:
        value = ++reg.X;
        break;
      case ABSOLUTE:
        value = getMem(ABS) + 1;
        writeMem_c1(ABS, value);
        break;
      case ZERO_PAGE_IDX:
        value = getMem(ZPG_X) + 1;
        writeMem_c1(ZPG_X, value);
        break;
      case ABSOLUTE_IDX:
        value = getMem(ABS_X) + 1;
        writeMem_c1(ABS_X, value);
        break;
      }
      reg.flags[N_f] = value & 0x80;
      reg.flags[Z_f] = value == 0x00;
      break;
    }
    }
  }
}

void CPU_6502::step(int nbSteps) {
  for (int i = 0; i < nbSteps; i++)
    this->step();
}

/******* Debug functions *******/

void CPU_6502::print_state() const {
  std::cout << "A=$" << print_hex(reg.A) << " X=$" << print_hex(reg.X) << " Y=$"
            << print_hex(reg.Y) << " PC=$" << print_hex(reg.PC) << " SP=$"
            << print_hex(reg.SP) << " flags=0b" << std::bitset<8>{42}
            << reg.flags << std::endl;
}

template <typename T> std::string CPU_6502::print_hex(T a) {
  std::stringstream ss;
  ss << std::setw(2 * sizeof(a)) << std::setfill('0') << std::hex << (int)a;
  return ss.str();
}

/******* Private functions *******/

uint16_t CPU_6502::getAddress_c1(uint8_t mode) {
  uint16_t result = 0x00;
  switch (mode) {
  case X_IND:
    // Pre-indexed Indirect : return the byte at address 0xYYXX where XX
    // is the byte stored at (operand+X) and YY the byte at (operand+X+1)
    result = ram->readByte(ram->readByte(reg.PC) + reg.X) +
             (ram->readByte(ram->readByte(reg.PC) + reg.X + 1) << 8);
    break;
  case ZPG:
    // Zero-page : pointer to address in the range 0x00 - 0xFF
    result = ram->readByte(reg.PC);
    break;
  case IMM:
    // Immediate : use operand as direct value
    result = reg.PC;
    break;
  case ABS:
    // Absolute : address specified by 2 operands
    result = ram->readByte(reg.PC) + (ram->readByte(reg.PC + 1) << 8);
    reg.PC++; // operand is 2 bytes long
    break;
  case IND_Y:
    // Indirect indexed : return the byte at Y-indexed address pointed by
    // the zero-page bytes at operand, operand+1
    result = reg.Y + ram->readByte(ram->readByte(reg.PC)) +
             (ram->readByte(ram->readByte(reg.PC) + 1) << 8);
    break;
  case ZPG_X:
    // X-Indexed zero page : return the 0-page byte at (operand+X)
    result = reg.X + ram->readByte(reg.PC);
    break;
  case ABS_Y:
    // Absolute indexed by Y : read 2-bytes address and index it by Y
    result = reg.Y + ram->readByte(reg.PC) + (ram->readByte(reg.PC + 1) << 8);
    break;
  case ABS_X:
    // Absolute indexed by X : read 2-bytes address and index it by X
    result = reg.X + ram->readByte(reg.PC) + (ram->readByte(reg.PC + 1) << 8);
    break;
  case ZPG_Y:
    // Y-Indexed zero page : return the 0-page byte at (operand+Y)
    result = reg.Y + ram->readByte(reg.PC);
    break;
  }

  reg.PC++;
  return result;
}

uint8_t CPU_6502::getMemAndIncrementPC(uint8_t mode) {
  return ram->readByte(getAddress_c1(mode));
}

uint8_t CPU_6502::getMem(uint8_t mode) {
  uint16_t PC_save = reg.PC;
  uint16_t address = getAddress_c1(mode);
  uint8_t value = ram->readByte(address);
  reg.PC = PC_save; // Restore PC
  return value;
}

void CPU_6502::writeMem_c1(uint8_t mode, uint8_t value) {
  if (mode == IMM) { // Can't write to an immediate value because it is
                     // not an address
    // Raise error
    return;
  }
  ram->writeByte(getAddress_c1(mode), value);
}