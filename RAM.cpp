#include "RAM.h"
#include <iostream>
#include <iomanip>

RAM::RAM(uint16_t  size) {
    this->memory = new char[size];
    std::cout << "RAM init !" << std::endl;
}

RAM::~RAM() {
    std::cout << "RAM destroyed" << std::endl;
}

void RAM::setByte(uint16_t address, char value) {
    this->memory[address] = value;
}

char RAM::getByte(uint16_t address) {
    return this->memory[address];
}

template<typename T>
std::string RAM::print_hex(T a, int size) {
    std::stringstream ss;
    ss << std::setw(size) << std::setfill('0') << std::hex << (int) a;
    return ss.str();
}

void RAM::printState(uint16_t start, uint16_t end) {
    for (uint16_t i=start;i<=end;i+=0x10) {
        std::cout << RAM::print_hex(i, 4) << ":\t";
        for (uint j=0;j<15;j++) {
            std::cout << RAM::print_hex(this->getByte(start+i+j),2) << " ";
        }
        std::cout << std::endl;
    }
}



