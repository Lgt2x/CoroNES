//
// Created by louis on 17/01/23.
//

#include "Cartridge.h"
#include <fstream>
#include <ios>
#include <vector>
#include <string>
#include <iomanip>
#include <iostream>


std::string print_hex(uint8_t a) {
    std::stringstream ss;
    ss << "0x" << std::setw(2*sizeof(a)) << std::setfill('0') << std::hex << (int) a;
    return ss.str();
}

void Cartridge::loadCart(const std::string& filename) {
    std::ifstream file(filename,std::ios_base::binary|std::ios_base::in);
    std::vector<uint8_t>header(0x10,0);
    file.read(reinterpret_cast<char*>(&header[0]),0x10);

    if (header[0] == 0x4e && header[1] == 0x45 && header[2] == 0x53 && header[3] == 0x1a) {
        std::cout << "NES file identified" << std::endl;
    }

    auto PRG_ROM_size = header[4];
    auto CHR_ROM_size = header[5];
    auto flags6 = header[6];
    auto flags7 = header[7];
    auto flags8 = header[8];
    auto flags9 = header[9];
    auto flags10 = header[10];

    uint8_t mapper = (flags6 >> 4) | (flags7 & 0xF0);
    std::cout << "PRG ROM size = " << print_hex(PRG_ROM_size) << " * 16kB\n";
    std::cout << "CHR ROM size = " << print_hex(CHR_ROM_size) << " * 8kB\n";
    std::cout << "Mapper :  " << print_hex(mapper) << std::endl;


    // Load PRG & CHR ROM
    prg_rom.resize(0x4000 * PRG_ROM_size,0);
    file.read(reinterpret_cast<char*>(&prg_rom[0]),0x4000 * PRG_ROM_size);

    chr_rom.resize(0x4000 * CHR_ROM_size,0);
    file.read(reinterpret_cast<char*>(&chr_rom[0]),0x4000 * CHR_ROM_size);
}

const std::vector<uint8_t>& Cartridge::getPRG_ROM() {
    return prg_rom;
}

const std::vector<uint8_t>& Cartridge::getCHR_ROM() {
    return chr_rom;
}

bool Cartridge::extended() {
    return prg_rom.size() == 0x8000;
}
