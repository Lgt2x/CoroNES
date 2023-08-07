#ifndef NES_MAPPER_H
#define NES_MAPPER_H


#include <utility>

#include "../Cartridge.h"

/**

https://www.nesdev.org/wiki/Board_table
 */
class Mapper {
public:
    explicit Mapper(Cartridge* cart): cart(cart) {};
    uint8_t readPRG(uint16_t address);
    void writePRG(uint16_t address);
private:
    Cartridge* cart;
};


#endif //NES_MAPPER_H
