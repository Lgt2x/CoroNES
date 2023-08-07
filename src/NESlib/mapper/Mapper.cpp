#include <iostream>
#include <iomanip>
#include "Mapper.h"


template<typename T>
std::string print_hex(T a, int size) {
    std::stringstream ss;
    ss << std::setw(size) << std::setfill('0') << std::hex << (int) a;
    return ss.str();
}

uint8_t Mapper::readPRG(const uint16_t address) {
    if (address <= 0xBFFF || address <= 0xFFFF && cart->extended()) {
        return cart->getPRG_ROM()[address - 0x8000];
    } else if (address >= 0xBFFF && !cart->extended()) { // Mirror $8000 for NROM-128
        return cart->getPRG_ROM()[(address - 0x8000)%0x4000];
    } else {
        std::cout << "illegal PRGROM access" << std::endl;
    }
    return 0x0;
}

void Mapper::writePRG(const uint16_t address) {
    std::cout << "Tried to write PRG ROM..." << std::endl;
}
