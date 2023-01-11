#include <iostream>
#include <memory>
#include <fstream>
#include "CPU.h"
#include "RAM.h"
#include "Parser.h"

int main() {
    auto ram = std::make_unique<RAM>(1000);
    auto cpu = std::make_unique<CPU_6502>(ram);

    std::ifstream code_file{"../asm/tests.asm"};
    if (code_file.is_open()) {
        std::string line;
        Instruction i{};
        while(std::getline(code_file,line)) {
            i = Parser::readLine(line);
            std::cout << "Read " << line << std::endl;
            cpu->execute(i);
            cpu->print_reg();

            ram->printState(0x00,0x10);
            std::cout << std::endl;
        }
    } else {
        std::cout << "Error : could not load ASM file" << std::endl;
    }

    return 0;
}
