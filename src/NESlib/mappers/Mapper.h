#pragma once

#include <cstdint>
#include <utility>

#include "../Cartridge.h"

/**

https://www.nesdev.org/wiki/Board_table
 */
class Mapper {
public:
    virtual uint8_t readPRG(uint16_t address) = 0;
    virtual void writePRG(uint16_t address, uint8_t value) = 0;
};