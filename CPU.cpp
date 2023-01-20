#include <iostream>
#include <iomanip>
#include <bitset>
#include "CPU.h"
#include "Bus.h"

CPU_6502::CPU_6502(std::unique_ptr<Bus>& ram) : A(0), X(0), Y(0), PC(0), flags(0), SP(0), ram(ram){
    std::cout << "CPU init !" << std::endl;
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
            return this->ram->readByte(address + this->X) + (this->ram->readByte(address + this->X + 1) >> 16);
        case INDIRECT_Y:
            return this->Y + this->ram->readByte(address) + (this->ram->readByte(address + 1) >> 16);
        case ABSOLUTE_INDIRECT:
            return this->ram->readByte(address) + this->ram->readByte(address + 1);
        default:
            return 0;
    }
}

char CPU_6502::getValue(int mode, uint16_t address) {
    if (mode == IMMEDIATE) {
        return (char) address;
    } else {
        return this->ram->readByte(this->getRAMAddress(mode, address));
    }
}

void CPU_6502::setRAMValue(uint16_t mode, uint16_t address, char byte) {
    this->ram->writeByte(getRAMAddress(mode, address), byte);
}

int CPU_6502::execute(const Instruction& i) {
//    switch (CPU_6502::mapCodes[i.operation]) {
//        case LDA:
//            this->i_LDA(i);
//            break;
//        case STA:
//            this->i_STA(i);
//            break;
//    }

    return 0;
}