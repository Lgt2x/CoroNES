#include "Parser.h"
#include <iostream>
#include <cstdint>
#include <regex>

Instruction Parser::readLine(const std::string& line) {
    std::istringstream address{line};
    std::string operation, value;
    address >> operation >> value;

    uint16_t addr = 0;
    uint16_t addressing_mode = 0;

    // Match addressing modes
    std:std::istringstream val{value};
    char ch;
    if (value.length() == 0) {
        addressing_mode = IMPLICIT;
        addr = 0;
    }
    else if (std::regex_match(value,std::regex("^#\\$[0-9a-fA-F]{2}$"))) {
        addressing_mode = IMMEDIATE;
        val >> ch >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\$[0-9a-fA-F]{4}$"))){
        addressing_mode = ABSOLUTE;
        val >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\$[0-9a-fA-F]{4},X$"))){
        addressing_mode = ABSOLUTE_X;
        val >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\$[0-9a-fA-F]{4},Y$"))){
        addressing_mode = ABSOLUTE_Y;
        val >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\$[0-9a-fA-F]{2}$"))){
        addressing_mode = ZERO_PAGE;
        val >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\$[0-9a-fA-F]{2},X$"))){
        addressing_mode = ZERO_PAGE_X;
        val >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\$[0-9a-fA-F]{2},Y$"))){
        addressing_mode = ZERO_PAGE_Y;
        val >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\(\\$[0-9a-fA-F]{2},X\\)$"))){
        addressing_mode = INDIRECT_X;
        val >> ch >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\(\\$[0-9a-fA-F]{2}\\),Y$"))){
        addressing_mode = INDIRECT_Y;
        val >> ch >> ch >> std::hex >> addr;
    } else if (std::regex_match(value,std::regex("^\\(\\$[0-9a-fA-F]{4}\\)$"))){
        addressing_mode = ABSOLUTE_INDIRECT;
        val >> ch >> ch >>std::hex >> addr;
    }

    return Instruction {
        operation,
        addressing_mode,
        addr
    };
}
