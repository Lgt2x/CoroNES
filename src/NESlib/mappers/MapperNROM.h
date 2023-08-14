#pragma once

#include <cstdint>
#include <utility>

#include "../Cartridge.h"
#include "mappers/Mapper.h"

/**

https://www.nesdev.org/wiki/Board_table
 */
class MapperNROM : public Mapper {
public:
    MapperNROM(Cartridge* cart): cart(cart) {};
    virtual uint8_t readPRG(uint16_t address);
    virtual void writePRG(uint16_t address, uint8_t value);
private:
    Cartridge* cart;
};
