#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>

#include "Bus.h"
#include "CPU.h"

enum opcode_c0_b0_implied { BRK, JSR, RTI, RTS };
enum opcode_c0_b2_implied { PHP, PLP, PHA, PLA, DEY, TAY, INY, INX };
enum opcode_c0_b6_implied { CLC, SEC, CLI, SEI, TYA, CLV, CLD, SED };
enum opcode_c0 { BIT = 1, JMP_abs, JMP_ind, STY, LDY, CPY, CPX };
enum opcode_c0_branch { BPL, BMI, BVC, BVS, BCC, BCS, BNE, BEQ };
enum opcode_c2 { ASL, ROL, LSR, ROR, STX, LDX, DEC, INC };
enum adressingModes_c1 { X_IND, ZPG, IMM, ABS, IND_Y, ZPG_X, ABS_Y, ABS_X };
enum opcode_c1 { ORA, AND, EOR, ADC, STA, LDA, CMP, SBC };
enum flags { N_f, V_f, B_f, D_f, I_f, Z_f, C_f };

/******* Public functions *******/
CPU_6502::CPU_6502(Bus *ram)
    : A(0), X(0), Y(0), PC(0x4000),
      flags({false, false, true, true, false, true, false, false}), SP(0xFD),
      ram(ram), reset_vector(0) {
  CPU_6502::reset();
}

void CPU_6502::reset() {
  reset_vector = ram->readByte(0xFFFC) | (ram->readByte(0xFFFD) << 8);
  PC = reset_vector;
  flags =
      std::vector<bool>{false, false, true, true, false, true, false, false};

}

void CPU_6502::step() {
  CPU_6502::print_state();

  // Read the opcode at the current program counter address and increment it
  uint8_t opcode = ram->readByte(PC);
  this->PC++;

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
      this->A |= this->getMem_c1(mode);
      this->setFlags(N_f | Z_f, A);
      break;
    case AND: // AND memory with accumulator
      this->A &= this->getMem_c1(mode);
      this->setFlags(N_f | Z_f, A);
      break;
    case EOR: // XOR memory with accumulator
      this->A ^= this->getMem_c1(mode);
      this->setFlags(N_f | Z_f, A);
      break;
    case ADC: { // Add memory to accumulator with carry
      uint8_t operand = getMem_c1(mode);
      uint16_t res = A + operand;
      this->setFlags(C_f, (res >> 8) > 0); // Set the carry
      // Set the overflow (V_f) flag. Is it set when the result changed the sign
      // bit when it should not have. e.g. when both input numbers have the sign
      // bit off but the result has the sign bit on. There is probably a more
      // efficient way to do this. See http://www.6502.org/tutorials/vflag.html
      // for full explanation
      this->setFlags(V_f, (A & 0x80 && operand & 0x80 && !(res & 0x80)) ||
                              (!(A & 0x80) && operand & 0x80 && res & 0x80));
      A = res & 0xFF;
      this->setFlags(N_f | Z_f, A);
      break;
    }
    case STA: // Store accumulator to memory
      this->writeMem_c1(mode, A);
      break;
    case LDA: // Load accumulator with memory
      A = this->getMem_c1(mode);
      setFlags(N_f | Z_f, A);
      break;
    case CMP: { // Compare memory with accumulator
      uint8_t operand = getMem_c1(mode);
      setFlags(C_f, A >= operand);
      setFlags(Z_f, A == operand);
      // set N_f ?
      break;  // auto bus = std::make_unique<Bus>(new DummyMapper());
    }
    case SBC: { // Subtract from accumulator with borrow
      uint8_t operand = getMem_c1(mode);
      uint16_t res = A - operand;// - flags[C_f];
      this->setFlags(C_f, (res >> 8) > 0); // Set the carry
      this->setFlags(V_f, (A & 0x80 && operand & 0x80 && !(res & 0x80)) ||
                              (!(A & 0x80) && operand & 0x80 && res & 0x80));
      A = res & 0xFF;
      this->setFlags(N_f | Z_f, A);
      break;
    }
    }
    break;
  case 0b10:
    std::cout << "Not supported yet." << std::endl;
    break;
  }
}

void CPU_6502::step(int nbSteps) {
    for (int i=0;i<nbSteps;i++) this->step();
}

/******* Debug functions *******/
std::vector<uint8_t> CPU_6502::dumpRegisters() {

  return std::vector<uint8_t>{this->A,
                              this->X,
                              this->Y,
                              this->SP,
                              CPU_6502::flagsToByte(),
                              static_cast<uint8_t>(this->PC & 0xFF),
                              static_cast<uint8_t>(this->PC >> 8)};
}

void CPU_6502::print_state() const {
  std::cout << "A=$" << print_hex(this->A) << " X=$" << print_hex(this->X)
            << " Y=$" << print_hex(this->Y) << " PC=$" << print_hex(this->PC)
            << " SP=$" << print_hex(this->SP) << " flags=0b"
            << std::bitset<8>{42} << (int)this->flagsToByte() << std::endl;
}

template <typename T> std::string CPU_6502::print_hex(T a) {
  std::stringstream ss;
  ss << std::setw(2 * sizeof(a)) << std::setfill('0') << std::hex << (int)a;
  return ss.str();
}

/******* Private functions *******/
uint8_t CPU_6502::flagsToByte() const {
  return std::accumulate(this->flags.begin(), this->flags.end(), 0,
                         [](bool a, bool b) { return (a << 1) + b; });
}

uint16_t CPU_6502::getAddress_c1(uint8_t mode) {
  uint16_t result = 0x00;
  switch (mode) {
  case X_IND:
    // Pre-indexed Indirect : return the byte at address 0xYYXX where XX is the byte
    // stored at (operand+X) and YY the byte at (operand+X+1)
    result = ram->readByte(ram->readByte(PC) + X) +
             (ram->readByte(ram->readByte(PC) + X + 1) << 8);
    break;
  case ZPG:
    // Zero-page : pointer to address in the range 0x00 - 0xFF
    result = ram->readByte(PC);
    break;
  case IMM:
    // Immediate : use operand as direct value
    result = PC;
    break;
  case ABS:
    // Absolute : address specified by 2 operands
    result = ram->readByte(PC) + (ram->readByte(PC + 1) << 8);
    PC++; // operand is 2 bytes long
    break;
  case IND_Y:
    // Indirect indexed : return the byte at Y-indexed address pointed by the
    // zero-page bytes at operand, operand+1
    result = Y + ram->readByte(ram->readByte(PC)) +
             (ram->readByte(ram->readByte(PC) + 1) << 8);
    break;
  case ZPG_X:
    // X-Indexed zero page : return the 0-page byte at (operand+X)
    result = X + ram->readByte(PC);
    break;
  case ABS_Y:
    // Absolute indexed by Y : read 2-bytes address and index it by Y
    result = Y + ram->readByte(PC) + (ram->readByte(PC + 1) << 8);
    break;
  case ABS_X:
    // Absolute indexed by X : read 2-bytes address and index it by X
    result = X + ram->readByte(PC) + (ram->readByte(PC + 1) << 8);
    break;
  }

  PC++;
  return result;
}

uint8_t CPU_6502::getMem_c1(uint8_t mode) {
  return ram->readByte(getAddress_c1(mode));
}

void CPU_6502::writeMem_c1(uint8_t mode, uint8_t value) {
  if (mode ==
      IMM) { // Can't write to an immediate value because it is not an address
    // Raise error
    return;
  }
  ram->writeByte(getAddress_c1(mode), value);
}

void CPU_6502::setFlags(uint8_t mask, uint8_t value) {
  if (mask & Z_f) { // Zero flag : set if result is null
    flags[Z_f] = value == 0;
  }
  if (mask & N_f) { // Negative flag : set if result has sign bit set
    flags[N_f] = value & 0x80;
  }
  if (mask & C_f) { // Carry flag : result is carried over more than 8 bits
    flags[C_f] = value;
  }
  if (mask & V_f) { // Overflow flag : result does not have the expected sign
    flags[V_f] = value;
  }
}