#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "mappers/Mapper.h"

/**
NES Memory bus, using 2-bytes adresses.

 Memory map
 --------------------------
 $0000 - $07FF      internal RAM
    $0000 - $00FF       zero page
    $0100 - $01FF       stack
    $0200 - $07FF       RAM
    $0800 - 1FFF        mirrors $0000 - $07FF
 $2000 - $3FFF      PPU registers
    $2000 - $2007       PPU registers
    $2008 - $3FFF       mirrors $2000 - $2007
 $4000 - $4017      APU & IO
 $4020 - $FFFF      Cartridge space, see mappers for details
    $FFFA - $FFFB       NMI Vector
    $FFFC - $FFFD       Reset Vector
    $FFFE - $FFFF       IRQ Vector

 */

class Bus {
public:
  Bus(Mapper* mapper); // optional mapper ?

  Bus(Bus& bus) = delete;
  ~Bus();

  uint8_t readByte(uint16_t address);
  void writeByte(uint16_t address, uint8_t value);

  template<typename T>static std::string print_hex(T a, int size);
  void printState(uint16_t start, uint16_t end);
private:
  std::vector<uint8_t>* ram;
  Mapper *mapper;
};
