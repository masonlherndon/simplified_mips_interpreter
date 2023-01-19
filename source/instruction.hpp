#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include "stage.hpp"

enum OPCODE {LW = 1, SW = 2, LI = 3,
             ADD = 4, ADDI = 5, MULT = 6, MULTI = 7, SUB = 8, SUBI = 9,
             BEQ = 10, BNE = 11, J = 12, HLT = 13};

struct Instruction{

    // set when parsed, they do not change
    int line_number = 0; // line counting from 1
    std::string original_line = ""; // original instruction like with minimal changes
    std::string label = ""; // alphabetical tag used to refernce lines of code
    std::string opcode = ""; // name of operation
    std::string arg1 = ""; // { X ,  ,  }
    std::string arg2 = ""; // {  , X ,  }
    std::string arg3 = ""; // {  ,  , X }
    std::string result_reg = ""; // location that result will be stored at
    std::string source_reg1 = ""; // first dependency (may or may not be a raw argument)
    std::string source_reg2 = ""; // second dependency (may or may not be a raw argument)

    // used to execute
    int instr_index = 0; // index of the instruction in the running_instr array
    int nearest_data_hazard = 0; // set to index of instruction that must be waited on (in IF), increment by 1 when that instruction moves forward (when pushing)
    int mem_count = 0;
    bool has_data_hazard = false; // set to true if a data hazard is detected (in IF), set back to false when nearest data hazard has cleared (when pushing)
    bool has_source_regs = false; // set to false if opcode is LI or J or HLT (during parsing)
    bool writes_to_register = false; // set to false for control opcodes and HLT (during parsing)
    bool already_wrote_result = false; // set to true if/when the instruction has written their result (during execution)
    bool is_branch = false; // set to true if opcode is BEQ or BNE (during parsing)
    bool has_completed = false; // set to true when instruction has entered FINISHED, (after pushing)
    bool exists = false; // set to false when line is empty, or just missing an opcode
    Stage in_stage = IF;
    int finish_log[NUM_STAGES];
};

#endif
