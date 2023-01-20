#include "Bus.h"
#include <iostream>
#include <iomanip>

Bus::Bus(Mapper* mapper) : mapper(mapper) {
    ram.resize(0x800,0);
    std::cout << "Bus init !" << std::endl;
}

Bus::~Bus() {
    std::cout << "Bus destroyed" << std::endl;
}

void Bus::writeByte(uint16_t address, uint8_t value) {
    if (address <= 0x1FFF) {
        // Internal RAM & mirrors
        ram[address%0x800] = value;
    }
}

uint8_t Bus::readByte(uint16_t address) {
    if (address <= 0x1FFF) {

        // Internal RAM & mirrors
        return ram[address%0x800];
    } else if (address <= 0x3FFF) {
        std::cout << "PPU Register accessed" << std::endl;
        return 0x0;
    } else if (address <= 0x401F) {
        std::cout << "APU || IO register accessed" << std::endl;
        return 0x0;
    } else {
        // Cartridge space
        return mapper->readPRG(address);
    }
}

template<typename T>
std::string Bus::print_hex(T a, int size) {
    std::stringstream ss;
    ss << std::setw(size) << std::setfill('0') << std::hex << (int) a;
    return ss.str();
}

void Bus::printState(uint16_t start, uint16_t end) {
    for (uint16_t i=start;i+0x10<=end;i+=0x10) {
        std::cout << Bus::print_hex(i, 4) << ":\t";
        for (uint j=0;j<15;j++) {
//            std::cout << (int) i << " " << (int) j << std::endl;
            std::cout << Bus::print_hex(readByte(i + j), 2) << " ";
        }
        std::cout << std::endl;
    }
}



