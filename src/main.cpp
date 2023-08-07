#include <memory>
#include <string>

#include "NESlib/CPU.h"
#include "NESlib/Bus.h"
#include "NESlib/Cartridge.h"
#include "NESlib/mapper/Mapper.h"

int main(int argc, char* argv[]) {
    std::string filename = std::string(argv[1]);
    Cartridge cart{filename};
    Mapper mapper(&cart);

    auto bus = std::make_unique<Bus>(&mapper);
    auto cpu = std::make_unique<CPU_6502>(bus);

    cpu.reset();

//    bus->printState(0x8000,0xFFF0);

    return 0;
}
