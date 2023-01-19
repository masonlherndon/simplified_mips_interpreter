#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <unordered_map>
#include "instruction.hpp"
#include "memory.hpp"
#include "flag_reg.hpp"

// used to confirm valid opcodes during parsing
// maps opcodes onto integer enums, improving readability
const std::unordered_map<std::string, OPCODE> VALID_OPCODES = {
    {"LW", LW}, {"SW", SW}, {"LI", LI},
    {"ADD", ADD}, {"ADDI", ADDI},
    {"MULT", MULT}, {"MULTI", MULTI},
    {"SUB", SUB}, {"SUBI", SUBI},
    {"BEQ", BEQ}, {"BNE", BNE}, {"J", J},
    {"HLT", HLT}};

// used to quickly convert string register operands into register indices
const std::unordered_map<std::string, int> VALID_REGISTERS = {
    {"R0", 0}, {"R1", 1}, {"R2", 2}, {"R3", 3},
    {"R4", 4}, {"R5", 5}, {"R6", 6}, {"R7", 7},
    {"R8", 8}, {"R9", 9}, {"R10", 10}, {"R11", 11},
    {"R12", 12}, {"R13", 13}, {"R14", 14}, {"R15", 15},
    {"R16", 16}, {"R17", 17}, {"R18", 18}, {"R19", 19},
    {"R20", 20}, {"R21", 21}, {"R22", 22}, {"R23", 23},
    {"R24", 24}, {"R25", 25}, {"R26", 26}, {"R27", 27},
    {"R28", 28}, {"R29", 29}, {"R30", 30}, {"R31", 31}};

// high level parsing functions
void load_program(Program& program, const std::string& filename);
void load_data(std::vector<std::bitset<REG_SIZE>>& data, const std::string& filename);

// helper parsing functions
void read_instruction_line(Instruction& instruction, std::string& line, const int line_number);
void extract_label(Instruction& instruction, std::string& line);
void extract_opcode(Instruction& instruction, std::string& line);
void extract_arguments(Instruction& instruction, std::string& line);
std::string extract_next_argument(std::string& line);
void fill_label_map(Program& program);
std::string get_register_from_address(const std::string& str);

// utility functions called in helper parsing functions
void remove_CR_and_LF(std::string& line);
void remove_bars(std::string& line);
void make_uppercase(std::string& line);
void trim_line(std::string& line);

// functions for running loaded program
void run_program(Memory& memory, const std::string& output_file);
Instruction get_next_filled_instruction(const std::vector<Instruction>& instructions, int& PC);
void update_flags(FlagReg& flags, std::vector<Instruction*>& active_instrs, const std::vector<Instruction>& running_instrs, const Stage stage);
void update_data_hazards(std::vector<Instruction*>& active_instrs, const std::vector<Instruction>& running_instrs);
void attempt_stage(Memory& memory, std::vector<std::bitset<REG_SIZE>>& registers, std::vector<Instruction*>& active_instrs, const std::vector<Instruction>& running_instrs, FlagReg& flags, const Stage stage, int& PC, const int cycle);
void attempt_push(std::vector<Instruction*>& active_instrs, const std::vector<Instruction>& running_instrs, FlagReg& flags, const Stage stage, const int cycle);

// output functions
void write_output(const std::vector<Instruction>& instruction_history, const std::string& output_file);
void print_output(const std::vector<Instruction>& instruction_history);

// utility functions for working with string operands
int get_value(const Memory& memory, const std::vector<std::bitset<REG_SIZE>>& registers, const std::string& str);
int resolve_address(const std::vector<std::bitset<REG_SIZE>>& data, const std::vector<std::bitset<REG_SIZE>>& registers, const std::string& str);
int address_to_index(const std::vector<std::bitset<REG_SIZE>>& registers, const std::string& str);
int resolve_immediate(const std::string& str);

// debugging functions
void print_instruction(const Instruction& instruction);
void print_instructions(const std::vector<Instruction>& instructions);
void print_registers(const std::vector<std::bitset<REG_SIZE>>& registers);
void examine_str(const std::string& name, const std::string& str);
void examine_str_ASCCI(const std::string& name, const std::string& str);

#endif
