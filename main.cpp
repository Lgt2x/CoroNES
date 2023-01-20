#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include "CPU.h"
#include "Bus.h"
#include "Parser.h"
#include "Cartridge.h"
#include "Mapper.h"

int main(int argc, char* argv[]) {
    Cartridge cart;
    cart.loadCart(std::string(argv[1]));
    Mapper mapper(&cart);

    auto bus = std::make_unique<Bus>(&mapper);
    auto cpu = std::make_unique<CPU_6502>(bus);

//    std::ifstream code_file{"../asm/tests.asm"};
//    if (code_file.is_open()) {
//        std::string line;
//        Instruction i{};
//        while(std::getline(code_file,line)) {
//            i = Parser::readLine(line);
//            std::cout << "Read " << line << std::endl;
//            cpu->execute(i);
//            cpu->print_reg();
//
//            ram->printState(0x00,0x10);
//            std::cout << std::endl;
//        }
//    } else {
//        std::cout << "Error : could not load ASM file" << std::endl;
//    }

    bus->printState(0x8000,0xFFFF);

    return 0;
}
