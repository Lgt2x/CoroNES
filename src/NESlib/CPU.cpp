#include <algorithm>
#include <bitset>
#include <csetjmp>
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
  nmi_vector = ram->readByte(0xFFFA) | (ram->readByte(0xFFFB) << 8);
  reset_vector = ram->readByte(0xFFFC) | (ram->readByte(0xFFFD) << 8);
  irq_vector = ram->readByte(0xFFFE) | (ram->readByte(0xFFFF) << 8);

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
  case 0b00: {
    if (mode == 0 && instruction <= 3) {
      switch (instruction) {
      case BRK:
        reg.PC += 1;
        ram->writeByte(reg.SP--, (reg.PC >> 8) & 0xFF);
        ram->writeByte(reg.SP--, (reg.PC & 0xFF));
        reg.flags[B_f] = true;
        ram->writeByte(reg.SP--, (uint8_t)(reg.flags.to_ulong()));
        reg.flags[I_f] = true;
        reg.PC = irq_vector;
        break;
      case JSR:
        break;
      case RTI:
        reg.flags = ram->readByte(++reg.SP);
        reg.flags[B_f] = false;
        reg.flags[I_f] = false;
        reg.PC = ram->readByte(++reg.SP) + (ram->readByte(++reg.SP) << 8);
        break;
      case RTS:
        break;
      }
    } else if (mode == 2) {
      switch (instruction) {
      case PHP:
        ram->writeByte(reg.SP, reg.flags.to_ulong() & 0xFF);
        --reg.SP;
        break;
      case PLP:
        ++reg.SP;
        reg.flags = ram->readByte(reg.SP);
        break;
      case PHA:
        ram->writeByte(reg.SP, reg.A);
        --reg.SP;
        break;
      case PLA:
        ++reg.SP;
        reg.A = ram->readByte(reg.SP);
        break;
      case DEY:
        --reg.Y;
        reg.flags[N_f] = reg.Y & 0x80;
        reg.flags[Z_f] = reg.Y == 0;
        break;
      case TAY:
        reg.Y = reg.A;
        reg.flags[N_f] = reg.A & 0x80;
        reg.flags[Z_f] = reg.A == 0;
        break;
      case INY:
        ++reg.Y;
        reg.flags[N_f] = reg.Y & 0x80;
        reg.flags[Z_f] = reg.Y == 0;
        break;
      case INX:
        ++reg.X;
        reg.flags[N_f] = reg.X & 0x80;
        reg.flags[Z_f] = reg.X == 0;
        break;
      }
    } else if (mode == 4) {
      bool doJump = false;
      switch (instruction) {
      case BPL:
        doJump = !reg.flags[N_f];
        break;
      case BMI:
        doJump = reg.flags[N_f];
        break;
      case BVC:
        doJump = !reg.flags[V_f];
        break;
      case BVS:
        doJump = reg.flags[V_f];
        break;
      case BCC:
        doJump = !reg.flags[C_f];
        break;
      case BCS:
        doJump = reg.flags[C_f];
        break;
      case BNE:
        doJump = !reg.flags[Z_f];
        break;
      case BEQ:
        doJump = reg.flags[Z_f];
        break;
      }

      if (doJump) {
        reg.PC += (int8_t)(ram->readByte(reg.PC));
      }

      reg.PC += 1;

    } else if (mode == 6) {
      switch (instruction) {
      case CLC:
        reg.flags[C_f] = false;
        break;
      case SEC:
        reg.flags[C_f] = true;
        break;
      case CLI:
        reg.flags[I_f] = false;
        break;
      case SEI:
        reg.flags[I_f] = true;
        break;
      case TYA:
        reg.A = reg.Y;
        reg.flags[N_f] = reg.A & 0x80;
        reg.flags[Z_f] = reg.A == 0;
        break;
      case CLV:
        reg.flags[V_f] = false;
        break;
      case CLD:
        reg.flags[D_f] = false;
        break;
      case SED:
        reg.flags[D_f] = true;
        break;
      }
    } else {
      enum adressingModes_c0 { ZPG_c0 = 1, ABS_c0 = 3, ZPG_X = 5, ABS_X = 7 };

      switch (instruction) {
      case BIT:
        break;
      case JMP_abs:
        break;
      case JMP_ind:
        break;
      case STY:
        break;
      case LDY:
        break;
      case CPY:
        break;
      case CPX:
        break;
      }
    }
    break;
  }
  case 0b01: {
    switch (instruction) {
    case ORA: // OR memory with accumulator
      reg.A |= this->readByteAndIncrementPC(mode);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    case AND: // AND memory with accumulator
      reg.A &= this->readByteAndIncrementPC(mode);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    case EOR: // XOR memory with accumulator
      reg.A ^= this->readByteAndIncrementPC(mode);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    case ADC: { // Add memory to accumulator with carry
      uint8_t operand = readByteAndIncrementPC(mode);
      uint16_t res = reg.A + operand;

      // The overflow (V_f) flag is set when the result changed the sign
      // bit when it should not have. e.g. when both input numbers have the sign
      // bit off but the result has the sign bit on.
      // See http://www.6502.org/tutorials/vflag.html
      reg.flags[V_f] = ((reg.A & 0x80) && (operand & 0x80) && !(res & 0x80)) ||
                       (!(reg.A & 0x80) && !(operand & 0x80) && res & 0x80);
      reg.flags[C_f] = res & 0x100;
      reg.flags[N_f] = res & 0x80;
      reg.flags[Z_f] = res == 0;

      reg.A = res & 0xFF;
      break;
    }
    case STA: // Store accumulator to memory
      this->writeByte(mode, reg.A);
      break;
    case LDA: // Load accumulator with memory
      reg.A = this->readByteAndIncrementPC(mode);
      reg.flags[N_f] = reg.A & 0x80;
      reg.flags[Z_f] = reg.A == 0;
      break;
    case CMP: { // Compare memory with accumulator
      uint8_t operand = readByteAndIncrementPC(mode);
      reg.flags[C_f] = reg.A >= operand;
      reg.flags[N_f] = reg.A & 0x80; // Not sure about that
      reg.flags[Z_f] = reg.A == operand;
      break;
    }
    case SBC: { // Subtract from accumulator with borrow
      uint8_t operand = readByteAndIncrementPC(mode);
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
  }
  case 0b10: {
    switch (instruction) {
    case ASL: {
      uint16_t value{};
      if (mode == IMM) {
        value = (reg.A << 1);
        reg.A = value & 0xFF;
      } else {
        uint16_t PC_save = reg.PC;
        value = (readByteAndIncrementPC(mode) << 1);
        reg.PC = PC_save;
        writeByte(mode, value & 0xFF);
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
        value = (readByteAndIncrementPC(mode) << 1) + reg.flags[C_f];
        reg.PC = PC_save;
        writeByte(mode, value & 0xFF);
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
        previous = readByteAndIncrementPC(mode);
        value = (previous >> 1);
        reg.PC = PC_save;
        writeByte(mode, value);
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
        previous = readByteAndIncrementPC(mode);
        value = (previous >> 1);
        reg.PC = PC_save;
        writeByte(mode, value);
      }
      reg.flags[N_f] = value & 0x80;
      reg.flags[Z_f] = value == 0x00;
      reg.flags[C_f] = previous & 0x01;
      break;
    }
    case STX: {
      switch (mode) {
      case ZERO_PAGE:
        writeByte(ZPG, reg.X);
        break;
      case IMPLIED:
        reg.A = reg.X;
        break;
      case ABSOLUTE:
        writeByte(ABS, reg.X);
        break;
      case ZERO_PAGE_IDX:
        writeByte(ZPG_Y, reg.X);
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
        reg.X = readByteAndIncrementPC(IMM);
        break;
      case ZERO_PAGE:
        reg.X = readByteAndIncrementPC(ZPG);
        break;
      case IMPLIED:
        reg.X = reg.A;
        break;
      case ABSOLUTE:
        reg.X = readByteAndIncrementPC(ABS);
        break;
      case ZERO_PAGE_IDX:
        reg.X = readByteAndIncrementPC(ZPG_Y);
        break;
      case STACK:
        reg.X = reg.SP;
        break;
      case ABSOLUTE_IDX:
        reg.X = readByteAndIncrementPC(ABS_Y);
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
        value = readByte(ZPG) - 1;
        writeByte(ZPG, value);
        break;
      case IMPLIED:
        value = --reg.X;
        break;
      case ABSOLUTE:
        value = readByte(ABS) - 1;
        writeByte(ABS, value);
        break;
      case ZERO_PAGE_IDX:
        value = readByte(ZPG_X) - 1;
        writeByte(ZPG_X, value);
        break;
      case ABSOLUTE_IDX:
        value = readByte(ABS_X) - 1;
        writeByte(ABS_X, value);
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
        value = readByte(ZPG) + 1;
        writeByte(ZPG, value);
        break;
      case IMPLIED:
        value = ++reg.X;
        break;
      case ABSOLUTE:
        value = readByte(ABS) + 1;
        writeByte(ABS, value);
        break;
      case ZERO_PAGE_IDX:
        value = readByte(ZPG_X) + 1;
        writeByte(ZPG_X, value);
        break;
      case ABSOLUTE_IDX:
        value = readByte(ABS_X) + 1;
        writeByte(ABS_X, value);
        break;
      }
      reg.flags[N_f] = value & 0x80;
      reg.flags[Z_f] = value == 0x00;
      break;
    }
    }
    break;
  }
  }
}

void CPU_6502::step(int nbSteps) {
  for (int i = 0; i < nbSteps; i++)
    this->step();
}

/******* Debug functions *******/

void CPU_6502::printState() const {
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

uint16_t CPU_6502::readAddressAndIncrementPC(uint8_t mode) {
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

uint8_t CPU_6502::readByteAndIncrementPC(uint8_t mode) {
  return ram->readByte(readAddressAndIncrementPC(mode));
}

uint8_t CPU_6502::readByte(uint8_t mode) {
  uint16_t PC_save = reg.PC;
  uint16_t address = readAddressAndIncrementPC(mode);
  uint8_t value = ram->readByte(address);
  reg.PC = PC_save; // Restore PC
  return value;
}

void CPU_6502::writeByte(uint8_t mode, uint8_t value) {
  if (mode == IMM) { // Can't write to an immediate value because it is
                     // not an address
    // Raise error
    return;
  }
  ram->writeByte(readAddressAndIncrementPC(mode), value);
}