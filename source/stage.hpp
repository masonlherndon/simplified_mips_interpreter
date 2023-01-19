#ifndef STAGE_HPP
#define STAGE_HPP

#include <string>

const int NUM_STAGES = 7;
const int NUM_STAGE_REGS = NUM_STAGES - 1;
const int NUM_MEM_CYCLES = 3;

enum Stage {IF = 0, ID = 1, EX1 = 2, EX2 = 3, EX3 = 4, MEM = 5, WB = 6};
const Stage stages[NUM_STAGES] = {IF, ID, EX1, EX2, EX3, MEM, WB};

#endif
