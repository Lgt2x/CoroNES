#include <iostream>
#include <iomanip>
#include <bitset>
#include "CPU.h"
#include "RAM.h"

CPU_6502::CPU_6502(std::unique_ptr<RAM>& ram) : A(0), X(0), Y(0), PC(0), flags(0), SP(0), ram(ram){
    this->init_codes();
    std::cout << "CPU init !" << std::endl;
}

void CPU_6502::init_codes() {
    CPU_6502::mapCodes["ADC"] = ADC;
    CPU_6502::mapCodes["AND"] = AND;
    CPU_6502::mapCodes["ASL"] = ASL;
    CPU_6502::mapCodes["BBC"] = BBC;
    CPU_6502::mapCodes["BCS"] = BCS;
    CPU_6502::mapCodes["BEQ"] = BEQ;
    CPU_6502::mapCodes["BIT"] = BIT;
    CPU_6502::mapCodes["BMI"] = BMI;
    CPU_6502::mapCodes["BNE"] = BNE;
    CPU_6502::mapCodes["BPL"] = BPL;
    CPU_6502::mapCodes["BRK"] = BRK;
    CPU_6502::mapCodes["BVC"] = BVC;
    CPU_6502::mapCodes["BVS"] = BVS;
    CPU_6502::mapCodes["CLC"] = CLC;
    CPU_6502::mapCodes["CLD"] = CLD;
    CPU_6502::mapCodes["CLI"] = CLI;
    CPU_6502::mapCodes["CLV"] = CLV;
    CPU_6502::mapCodes["CMP"] = CMP;
    CPU_6502::mapCodes["CPX"] = CPX;
    CPU_6502::mapCodes["CPY"] = CPY;
    CPU_6502::mapCodes["DEC"] = DEC;
    CPU_6502::mapCodes["DEX"] = DEX;
    CPU_6502::mapCodes["DEY"] = DEY;
    CPU_6502::mapCodes["EOR"] = EOR;
    CPU_6502::mapCodes["INC"] = INC;
    CPU_6502::mapCodes["INX"] = INX;
    CPU_6502::mapCodes["INY"] = INY;
    CPU_6502::mapCodes["JMP"] = JMP;
    CPU_6502::mapCodes["JSR"] = JSR;
    CPU_6502::mapCodes["LDA"] = LDA;
    CPU_6502::mapCodes["LDX"] = LDX;
    CPU_6502::mapCodes["LDY"] = LDY;
    CPU_6502::mapCodes["LSR"] = LSR;
    CPU_6502::mapCodes["NOP"] = NOP;
    CPU_6502::mapCodes["ORA"] = ORA;
    CPU_6502::mapCodes["PHA"] = PHA;
    CPU_6502::mapCodes["PHP"] = PHP;
    CPU_6502::mapCodes["PLA"] = PLA;
    CPU_6502::mapCodes["PLP"] = PLP;
    CPU_6502::mapCodes["ROL"] = ROL;
    CPU_6502::mapCodes["ROR"] = ROR;
    CPU_6502::mapCodes["RTI"] = RTI;
    CPU_6502::mapCodes["RTS"] = RTS;
    CPU_6502::mapCodes["SBC"] = SBC;
    CPU_6502::mapCodes["SEC"] = SEC;
    CPU_6502::mapCodes["SED"] = SED;
    CPU_6502::mapCodes["SEI"] = SEI;
    CPU_6502::mapCodes["STA"] = STA;
    CPU_6502::mapCodes["STX"] = STX;
    CPU_6502::mapCodes["STY"] = STY;
    CPU_6502::mapCodes["TAX"] = TAX;
    CPU_6502::mapCodes["TAY"] = TAY;
    CPU_6502::mapCodes["TSX"] = TSX;
    CPU_6502::mapCodes["TXA"] = TXA;
    CPU_6502::mapCodes["TXS"] = TXS;
    CPU_6502::mapCodes["TYA"] = TYA;
}

void CPU_6502::print_reg() const {
    std::cout << "A=$" << print_hex(this->A)
        << " X=$" << print_hex(this->X)
        << " Y=$" << print_hex(this->Y)
        << " PC=$" << print_hex(this->PC)
        << " SP=$" << print_hex(this->SP)
        << " flags=0b" << std::bitset<8>{42} << (int) this->flags << std::endl;
}

template<typename T>
std::string CPU_6502::print_hex(T a) {
    std::stringstream ss;
    ss << std::setw(2*sizeof(a)) << std::setfill('0') << std::hex << (int) a;
    return ss.str();
}

uint16_t CPU_6502::getRAMAddress(int mode, uint16_t address) {
    switch (mode) {
        case ABSOLUTE:
            return address;
        case ABSOLUTE_X:
            return address + this->X;
        case ABSOLUTE_Y:
            return address + this->Y;
        case ZERO_PAGE:
            return address;
        case ZERO_PAGE_X:
            return address + this->X;
        case ZERO_PAGE_Y:
            return address + this->Y;
        case INDIRECT_X:
            return this->ram->getByte(address + this->X) + (this->ram->getByte(address + this->X + 1) >> 16);
        case INDIRECT_Y:
            return this->Y + this->ram->getByte(address) + (this->ram->getByte(address + 1) >> 16);
        case ABSOLUTE_INDIRECT:
            return this->ram->getByte(address) + this->ram->getByte(address+1);
        default:
            return 0;
    }
}

char CPU_6502::getValue(int mode, uint16_t address) {
    if (mode == IMMEDIATE) {
        return (char) address;
    } else {
        return this->ram->getByte(this->getRAMAddress(mode, address));
    }
}

void CPU_6502::setRAMValue(uint16_t mode, uint16_t address, char byte) {
    this->ram->setByte(getRAMAddress(mode, address), byte);
}



int CPU_6502::execute(const Instruction& i) {
    switch (CPU_6502::mapCodes[i.operation]) {
        case LDA:
            this->i_LDA(i);
            break;
        case STA:
            this->i_STA(i);
            break;
    }

    return 0;
}


// Operation implementation
void CPU_6502::i_LDA(const Instruction& i) {
    this->A = this->getValue(i.addressing_mode, i.value);
}

void CPU_6502::i_STA(const Instruction& i) {
    this->setRAMValue(i.addressing_mode,i.value, this->A);
}

void CPU_6502::i_TAX(const Instruction& i) {
    this->X = this->A;
}

void CPU_6502::i_TAY(const Instruction& i) {
    this->Y = this->A;
}