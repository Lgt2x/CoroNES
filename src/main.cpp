#include <memory>
#include <string>

#include "CPU.h"
#include "Bus.h"
#include "Cartridge.h"
#include "mapper/MapperNROM.h"

int main(int argc, char* argv[]) {
    std::string filename = std::string(argv[1]);
    Cartridge cart{filename};
    MapperNROM mapper(&cart);

    auto bus = std::make_unique<Bus>(&mapper);
    auto cpu = std::make_unique<CPU_6502>(bus.get());

    cpu.reset();

//    bus->printState(0x8000,0xFFF0);

    return 0;
}
