#pragma once

#include <tuple>
#include <string>
#include <cstdint>
#include <map>

struct Instruction {
    std::string operation;
    uint16_t addressing_mode;
    uint16_t value;
};

enum Addressing_modes {
  IMPLICIT,
  IMMEDIATE,
  ABSOLUTE,
  ABSOLUTE_X,
  ABSOLUTE_Y,
  ZERO_PAGE, // Or Relative
  ZERO_PAGE_X,
  ZERO_PAGE_Y,
  INDIRECT_X,
  INDIRECT_Y,
  ABSOLUTE_INDIRECT,
};

class Parser {
public:
  static Instruction readLine(const std::string& line);
};

