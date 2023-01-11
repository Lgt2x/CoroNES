#pragma once

#include <vector>
#include <cstdint>
#include <string>

class RAM {
public:
  explicit RAM(uint16_t size);
  RAM(RAM& ram) = delete;
  ~RAM();

  char getByte(uint16_t address);
  void setByte(uint16_t address, char value);

  template<typename T>static std::string print_hex(T a, int size);
  void printState(uint16_t start, uint16_t end);
private:
  char* memory;
};
