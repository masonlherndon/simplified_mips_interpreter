#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <vector>
#include <bitset>
#include <unordered_map>
#include "program.hpp"

// const int OPTIONAL_CYCLE_LIMIT = 100;
const int REG_SIZE = 32;
const int NUM_REGS = 32;

struct Memory{
    // program loaded from input file
    Program program;

    // data from the data file, still in bits
    // filled when data file is parsed, changes during runtime
    std::vector<std::bitset<REG_SIZE>> data;
};

#endif
