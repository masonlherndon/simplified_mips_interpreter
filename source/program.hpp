#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <vector>
#include <unordered_map>
#include "instruction.hpp"

struct Program {
    // instructions from the program input file
    // filled when instruction file is parsed, does not change during runtime
    std::vector<Instruction> instructions;

    // map of each string label to the line number of its instruction
    // filled when instruction file is parsed, does not change during runtime
    std::unordered_map<std::string, int> labels;
};

#endif
