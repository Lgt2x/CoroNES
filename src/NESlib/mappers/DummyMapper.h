#pragma once

#include "mappers/Mapper.h"
#include <cstdint>
#include <vector>

class DummyMapper : public Mapper {
public:
    DummyMapper() {
      memory.resize(0xFFFF-0xBFFF);
    }

    uint8_t readPRG(uint16_t address) { 
      return memory.at(address-0xBFFF);
     };
    void writePRG(uint16_t address, uint8_t value) {
      memory.at(address-0xBFFF) = value;
    };
private:
  std::vector<uint8_t> memory;
};

