#include <iomanip>
#include <iostream>
#include "Bus.h"

Bus::Bus(Mapper *mapper) : mapper(mapper) {
  ram.resize(0xFFFF + 1, 0);
}

void Bus::writeByte(uint16_t address, uint8_t value) {
  if (address <= 0x1FFF) {
    // Internal RAM & mirrors
    ram.at(address % 0x800) = value;
  } else {
    mapper->writePRG(address, value);
  }
}

uint8_t Bus::readByte(uint16_t address) {
  if (address <= 0x1FFF) {
    // Internal RAM & mirrors
    return ram[address % 0x800];
  } else if (address <= 0x3FFF) {
    std::cout << "PPU Register accessed" << std::endl;
    return 0x0;
  } else if (address <= 0x401F) {
    std::cout << "APU || IO register accessed" << std::endl;
    return 0x0;
  } else {
    // Cartridge space, defer to the mapper
    return mapper->readPRG(address);
  }
}

template <typename T> std::string Bus::print_hex(T a, int size) {
  std::stringstream ss;
  ss << std::setw(size) << std::setfill('0') << std::hex << (int)a;
  return ss.str();
}

void Bus::printState(uint16_t start, uint16_t end) {
  for (int i = start; i <= end; i += 0x10) {
    std::cout << Bus::print_hex(i, 4) << ":\t";
    for (uint8_t j = 0; j <= 15; j++) {
      if (j == 8) {
        std::cout << "  ";
      }
      std::cout << Bus::print_hex(readByte(i + j), 2) << " ";
    }
    std::cout << std::endl;
  }
}
