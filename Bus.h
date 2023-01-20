#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "Mapper.h"


class Bus {
public:
  explicit Bus(Mapper* mapper);
  Bus(Bus& bus) = delete;
  ~Bus();

  uint8_t readByte(uint16_t address);
  void writeByte(uint16_t address, uint8_t value);

  template<typename T>static std::string print_hex(T a, int size);
  void printState(uint16_t start, uint16_t end);
private:
  std::vector<uint8_t> ram;
  Mapper *mapper;
};
