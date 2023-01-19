#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>
#include "utils.hpp"
#include "memory.hpp"
using std::bitset;
using std::cout;
using std::endl;
using std::fstream;
using std::ofstream;
using std::string;
using std::to_string;
using std::vector;

extern const bool ENABLE_UNLIMITED_INPUT;
extern const int OPTIONAL_CYCLE_LIMIT;
extern const int MAX_CYCLE_LIMIT;
const string WHITESPACE = " \n\t\r\f\v";    // for trimming whitespace (might use isspace() instead)
const int NUM_PAD_SPACES = 35;

// fills instruction memory from input file
void load_program(Program& program, const string& filename)
{
    string line;
    fstream file;
    int num_instrs = 0;

    // load each instruction into memory, line by line
    file.open(filename);
    if (file.is_open())
    {

        // count lines so we can presize instruction vector
        while (!file.eof())
        {
            file.ignore(256, '\n');
            num_instrs++;
        }
        program.instructions.resize(num_instrs);

        // go back to the start of file
        file.seekg(0);

        // parse each line
        for (int ii = 0; ii < num_instrs; ii++)
        {
            getline(file, line);
            read_instruction_line(program.instructions[ii], line, ii + 1);
        }
        file.close();

        fill_label_map(program);
    }
    else
    {
        cout << "File could not be opened!" << endl;
    }
}

// completely parses a single instruction
void read_instruction_line(Instruction& instruction, string& line, const int line_number)
{
    // clean up line
    remove_bars(line);
    remove_CR_and_LF(line);

    // store trivial variables
    instruction.line_number = line_number;
    instruction.original_line = line;

    // standardize line to be like native MIPS
    trim_line(line);
    make_uppercase(line);

    // parse rest of instruction member variables
    extract_label(instruction, line);
    extract_opcode(instruction, line);
    extract_arguments(instruction, line);
}

// gets rid of "|" so that accepted input can still slightly resemble native MIPS
void remove_bars(string& line)
{
    for (unsigned int ii = 0; ii < line.length(); ii++)
    {
        if (line[ii] == '|')
        {
            line.erase(ii, 1);
            ii--;
        }
    }
}

void remove_CR_and_LF(std::string& line)
{
    for (unsigned int ii = 0; ii < line.length(); ii++)
    {
        if (line[ii] == '\n' || line[ii] == '\r')
        {
            line.replace(ii, 1, " ");
        }
    }
}

// removes leading and trailing whitespaces of given string to aid parsing
void trim_line(string& line)
{
    if (not line.empty())
    {

        // calculate inclusive range
        int first_non_whitespace = line.find_first_not_of(WHITESPACE);
        int last_non_whitespace = line.find_last_not_of(WHITESPACE);

        // trim line being parsed
        line = line.substr(first_non_whitespace, last_non_whitespace - first_non_whitespace + 1);
    }
}

// makes every alphabetical character uppercase to remove case sensitivity
void make_uppercase(string& line)
{
    for (unsigned int ii = 0; ii < line.length(); ii++)
    {
        line[ii] = toupper(line[ii]);
    }
}

// parses label without using "|"
void extract_label(Instruction& instruction, string& line)
{
    int pos = line.find(':');
    if (pos != -1)
    {    // label exists

        // store label and remove from line being parsed
        instruction.label = line.substr(0, pos);
        line.erase(0, pos + 1);
        trim_line(line);
    }
    else
    {    // label does not exist
        instruction.label = "";
    }
}

// parses opcode without using "|"
void extract_opcode(Instruction& instruction, string& line)
{
    if (line.empty())
    {
        instruction.opcode = "";
        // instruction.is_empty = true;
        instruction.exists = false;
    }
    else
    {
        instruction.exists = true;
    }

    unsigned int cur_length = 0;
    string cur_op = "";
    string next_op = "";
    bool cur_valid;
    bool next_valid;

    // keep widening search until cur is valid, but next is invalid
    do
    {
        // get potential opcodes
        cur_op = line.substr(0, cur_length);
        if (cur_length + 1 <= line.length())
        {
            next_op = line.substr(0, cur_length + 1);
        }

        // check if potential opcodes are valid
        cur_valid = VALID_OPCODES.contains(cur_op);
        next_valid = VALID_OPCODES.contains(next_op);
        cur_length++;

    } while ((cur_valid == false || next_valid == true) && cur_length <= line.length());

    // store opcode and remove from line being parsed
    instruction.opcode = cur_op;
    line.erase(0, cur_length - 1);
    trim_line(line);
}

// assigns {arg1, arg2, arg3} for each instruction given its comma seperated operands
void extract_arguments(Instruction& instruction, string& line)
{
    string arg1 = extract_next_argument(line);
    string arg2 = extract_next_argument(line);
    string arg3 = extract_next_argument(line);
    instruction.arg1 = arg1;
    instruction.arg2 = arg2;
    instruction.arg3 = arg3;

    int opcode = VALID_OPCODES.at(instruction.opcode);
    switch (opcode)
    {
    case LW:    // {rd, #(rs)}
        instruction.result_reg = arg1;
        instruction.source_reg1 = get_register_from_address(arg2);
        instruction.writes_to_register = true;
        instruction.has_source_regs = true;
        break;

    case SW:    // {rs, #(rt)}
        instruction.source_reg1 = arg1;
        instruction.source_reg2 = get_register_from_address(arg2);
        instruction.has_source_regs = true;
        break;

    case LI:    // {rd, #}
        instruction.result_reg = arg1;
        instruction.writes_to_register = true;
        break;

    case ADD:     // {rd, rs, rt}
    case MULT:    // {rd, rs, rt}
    case SUB:     // {rd, rs, rt}
        instruction.result_reg = arg1;
        instruction.source_reg1 = arg2;
        instruction.source_reg2 = arg3;
        instruction.writes_to_register = true;
        instruction.has_source_regs = true;
        break;

    case ADDI:     // {rd, rs, #}
    case MULTI:    // {rd, rs, #}
    case SUBI:     // {rd, rs, #}
        instruction.result_reg = arg1;
        instruction.source_reg1 = arg2;
        instruction.writes_to_register = true;
        instruction.has_source_regs = true;
        break;

    case BEQ:    // {rs, rt, label/#}
    case BNE:    // {rs, rt, label/#}
        instruction.source_reg1 = arg1;
        instruction.source_reg2 = arg2;
        instruction.has_source_regs = true;
        instruction.is_branch = true;
        break;

    case J:      // {label/#}
    case HLT:    // {}
        break;
    };
}

// extracts the next argument from comma seperated list of operands
// returns empty string if all operands have already been extracted
// use to generalise operations with the same number of operands
string extract_next_argument(string& line)
{
    // remove spaces to widen scope of valid input (allows spaces in arg list)
    trim_line(line);

    string next_arg;
    int comma_pos = line.find(',');

    if (line.empty())
    {
        return "";
    }
    else if (comma_pos != -1)
    {    // comma exists (at least 2 arguments left)
        next_arg = line.substr(0, comma_pos);
        line.erase(0, comma_pos + 1);
        return next_arg;
    }
    else
    {    // comma does not exist (only 1 argument left)
        next_arg = line.substr(0, comma_pos);
        line.erase(0, line.length());
        return next_arg;
    }
}

string get_register_from_address(const string& str)
{
    string copy = str;

    int open_par_pos = str.find('(');
    string op1 = str.substr(0, open_par_pos);

    copy.erase(0, open_par_pos + 1);
    int close_par_pos = copy.find(')');
    string op2 = copy.substr(0, close_par_pos);

    if (str[0] != 'R')
    {    // op1 is immediate, op2 is register
        return op2;
    }
    else
    {    // op1 is register, op2 is immediate
        return op1;
    }
}

// put labels into hashmap, if instruction doesnt have a label use its line number
void fill_label_map(Program& program)
{
    string label;
    int line_number;

    for (unsigned int ii = 0; ii < program.instructions.size(); ii++)
    {
        label = program.instructions[ii].label;
        line_number = program.instructions[ii].line_number;

        // add every line number as a label, and the actual label if one exists
        program.labels.insert({to_string(line_number), line_number});
        if (not label.empty())
        {
            program.labels.insert({label, line_number});
        }
    }
}

// fills data memory from input file
void load_data(vector<bitset<REG_SIZE>>& data, const string& filename)
{
    string line;
    fstream file;
    int num_words = 0;    // each line is a word (32 bits)

    // load each instruction into memory, line by line
    file.open(filename);
    if (file.is_open())
    {
        // count lines so we can presize data vector
        while (!file.eof())
        {
            file.ignore(256, '\n');
            num_words++;
        }
        data.resize(num_words);

        // go back to the start of file
        file.seekg(0);

        // parse each line
        for (int ii = 0; ii < num_words; ii++)
        {
            file >> data[ii];
        }

        file.close();
    }
    else
    {
        cout << "File could not be opened!" << endl;
    }
}

void run_program(Memory& memory, const string& output_file)
{
    vector<Instruction*> active_instrs = vector<Instruction*>(7, nullptr);
    FlagReg flags;
    int PC = 0;
    int cycle = 1;

    vector<bitset<REG_SIZE>> registers = vector<bitset<REG_SIZE>>(NUM_REGS, 0);
    vector<Instruction> running_instrs;
    running_instrs.reserve(MAX_CYCLE_LIMIT);

    while (not flags.program_complete)
    {

        // try IF, other instructions will have been pushed forward if the flag is set
        update_flags(flags, active_instrs, running_instrs, IF);
        if (flags.able_to_insert)
        {

            // put new instruction into running_instrs
            running_instrs.push_back(get_next_filled_instruction(memory.program.instructions, PC));

            // put pointer to new instruction into array of active instrs (basically stage regs)
            running_instrs.back().instr_index = running_instrs.size() - 1;
            running_instrs.back().in_stage = IF;
            active_instrs[IF] = &running_instrs.back();
            active_instrs[IF]->finish_log[IF] = cycle;
        }

        // finish/execute each stage, move it to the next stage
        for (int ii = NUM_STAGES - 1; ii >= 0; ii--)
        {
            update_flags(flags, active_instrs, running_instrs, IF);

            // the instruction exists
            if (active_instrs[ii])
            {
                attempt_stage(memory, registers, active_instrs, running_instrs, flags, stages[ii], PC, cycle);
            }
        }

        // increment PC and terminate program if limit has been reached or instructions have finished
        cycle++;
        if (not ENABLE_UNLIMITED_INPUT && cycle > OPTIONAL_CYCLE_LIMIT)
        {
            // check if limit has been reached
            flags.program_complete = true;
        }
    }

    write_output(running_instrs, output_file);
    print_output(running_instrs);
}

void update_flags(FlagReg& flags, vector<Instruction*>& active_instrs, const vector<Instruction>& running_instrs, const Stage stage)
{
    update_data_hazards(active_instrs, running_instrs);

    // set finishing_up
    if (active_instrs[IF] && active_instrs[IF]->exists && active_instrs[IF]->opcode == "HLT")
    {
        flags.finishing_up = true;
    }

    // set control_hazard_exists and able_to_insert
    flags.control_hazard_exists = active_instrs[ID] && active_instrs[ID]->exists && active_instrs[ID]->is_branch;
    flags.able_to_insert = active_instrs[IF] && not active_instrs[IF]->exists && not flags.control_hazard_exists && not flags.finishing_up;

    if (not active_instrs[IF] && not flags.finishing_up)
    {
        if (not active_instrs[IF] && not flags.control_hazard_exists && not flags.finishing_up)
        {
            flags.able_to_insert = true;
        }
        return;
    }

    // set susceptible_to_data_hazard
    bool susceptible_to_data_hazard = false;
    if (stage == IF)
    {
        if (active_instrs[stage] && active_instrs[stage]->is_branch)
        {
            susceptible_to_data_hazard = true;
        }
    }
    else if (stage == ID)
    {
        if (active_instrs[stage] && active_instrs[stage]->has_source_regs && not active_instrs[stage]->is_branch)
        {
            susceptible_to_data_hazard = true;
        }
    }

    // set has_structural_hazard and able_to_push
    if (stage == WB)
    {
        flags.has_structural_hazard = false;
        flags.able_to_push = true;
    }
    else
    {

        flags.has_data_hazard = active_instrs[stage] && active_instrs[stage]->has_data_hazard;

        flags.has_structural_hazard = active_instrs[stage + 1] && active_instrs[stage + 1]->exists;
        flags.able_to_push = not flags.has_structural_hazard && (not susceptible_to_data_hazard || not flags.has_data_hazard);

        if (stage == MEM && active_instrs[stage] && active_instrs[stage] && active_instrs[stage]->mem_count < 3)
        {
            flags.able_to_push = false;
        }
    }

    // set is_stalling
    if (not flags.has_structural_hazard && flags.has_data_hazard)
    {
        flags.is_stalling = true;
    }
    else
    {
        flags.is_stalling = false;
    }

    // set finish_op_this_stage
    if (stage == ID)
    {
        if (active_instrs[stage] && active_instrs[stage]->opcode == "HLT")
        {
            flags.finish_op_this_stage = true;
        }
    }
    else if (stage == EX1)
    {
        if (active_instrs[stage] && (active_instrs[stage]->opcode == "J" || active_instrs[stage]->opcode == "BEQ" || active_instrs[stage]->opcode == "BNE"))
        {
            flags.finish_op_this_stage = true;
            active_instrs[stage]->finish_log[EX3] = active_instrs[stage]->finish_log[ID] + 1;
        }
    }
    else if (stage == WB)
    {
        flags.finish_op_this_stage = true;
    }
    else
    {
        flags.finish_op_this_stage = false;
    }

    // set program_complete
    if (active_instrs[ID] && active_instrs[ID]->exists && active_instrs[ID]->opcode == "HLT")
    {
        flags.program_complete = active_instrs[EX1] && active_instrs[EX1]->has_completed && active_instrs[EX2] && active_instrs[EX2]->has_completed && active_instrs[EX3] && active_instrs[EX3]->has_completed && active_instrs[MEM] && active_instrs[MEM]->has_completed && active_instrs[WB] && active_instrs[WB]->has_completed;
    }
}

Instruction get_next_filled_instruction(const vector<Instruction>& instructions, int& PC)
{
    Instruction cur_instr;
    if ((unsigned int)PC > instructions.size() - 1)
    {
        cout << "ERROR: PC is out of bounds!" << endl;
        return cur_instr;
    }
    else
        while ((unsigned int)PC < instructions.size())
        {
            cur_instr = instructions[PC];
            PC++;

            if (cur_instr.exists)
            {
                return cur_instr;
            }
        }
    return cur_instr;
}

void attempt_stage(Memory& memory, vector<bitset<REG_SIZE>>& registers, vector<Instruction*>& active_instrs, const vector<Instruction>& running_instrs, FlagReg& flags, Stage stage, int& PC, const int cycle)
{
    string arg1 = active_instrs[stage]->arg1;
    string arg2 = active_instrs[stage]->arg2;
    string arg3 = active_instrs[stage]->arg3;

    update_flags(flags, active_instrs, running_instrs, stage);
    int opcode = VALID_OPCODES.at(active_instrs[stage]->opcode);
    switch (opcode)
    {
    case LW:    // {rd, #(rs)}
        if (stage == WB)
        {
            registers[VALID_REGISTERS.at(arg1)] = get_value(memory, registers, arg2);
            active_instrs[stage]->already_wrote_result = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case SW:    // {rs, #(rt)}
        if (stage == MEM && active_instrs[stage]->mem_count >= 3)
        {
            memory.data[address_to_index(registers, arg2)] = get_value(memory, registers, arg1);
            active_instrs[stage]->already_wrote_result = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case LI:    // {rd, #}
        if (stage == WB)
        {
            registers[VALID_REGISTERS.at(arg1)] = resolve_immediate(arg2);
            active_instrs[stage]->already_wrote_result = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case ADD:     // {rd, rs, rt}
    case ADDI:    // {rd, rs, #}
        if (stage == WB)
        {
            registers[VALID_REGISTERS.at(arg1)] = get_value(memory, registers, arg2) + get_value(memory, registers, arg3);
            active_instrs[stage]->already_wrote_result = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case MULT:     // {rd, rs, rt}
    case MULTI:    // {rd, rs, #}
        if (stage == WB)
        {
            registers[VALID_REGISTERS.at(arg1)] = get_value(memory, registers, arg2) * get_value(memory, registers, arg3);
            active_instrs[stage]->already_wrote_result = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case SUB:     // {rd, rs, rt}
    case SUBI:    // {rd, rs, #}
        if (stage == WB)
        {
            registers[VALID_REGISTERS.at(arg1)] = get_value(memory, registers, arg2) - get_value(memory, registers, arg3);
            active_instrs[stage]->already_wrote_result = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case BEQ:    // {rs, rt, label/#}
        if (stage == ID)
        {
            if (get_value(memory, registers, arg1) == get_value(memory, registers, arg2))
            {
                PC = memory.program.labels.at(arg3);
                active_instrs[stage]->finish_log[EX3] = active_instrs[stage]->finish_log[EX1] + 1;
            }
        }
        else if (stage == EX1)
        {
            active_instrs[stage]->has_completed = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case BNE:    // {rs, rt, label/#}
        if (stage == ID)
        {
            if (get_value(memory, registers, arg1) != get_value(memory, registers, arg2))
            {
                PC = memory.program.labels.at(arg3) - 1;
                active_instrs[stage]->finish_log[EX3] = active_instrs[stage]->finish_log[EX1] + 1;
            }
        }
        else if (stage == EX1)
        {
            active_instrs[stage]->has_completed = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case J:    // {label/#}
        if (stage == ID)
        {
            PC = memory.program.labels.at(arg3) - 1;
            active_instrs[stage]->finish_log[EX3] = active_instrs[stage]->finish_log[EX1] + 1;
        }
        else if (stage == EX1)
        {
            active_instrs[stage]->has_completed = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;

    case HLT:    // {}
        if (stage == EX1)
        {
            active_instrs[stage]->has_completed = true;
        }
        attempt_push(active_instrs, running_instrs, flags, stage, cycle);
        break;
    };
}

// returns the integer value of any argument
int get_value(const Memory& memory, const vector<bitset<REG_SIZE>>& registers, const string& str)
{
    int left_pos = str.find('(');
    int right_pos = str.find(')');

    if (VALID_REGISTERS.contains(str))
    {    // its a register
        return registers.at(VALID_REGISTERS.at(str)).to_ullong();
    }
    else if (left_pos != -1 && right_pos != -1)
    {    // its an address
        return resolve_address(memory.data, registers, str);
    }
    else if (memory.program.labels.contains(str))
    {    // its a label (happens to handle some immediates)
        return memory.program.labels.at(str);
    }
    else
    {    // its an immediate
        return resolve_immediate(str);
    }
}

// calculates the address given by register/offset pairs
// use for instructions that access data memory
int resolve_address(const vector<bitset<REG_SIZE>>& data, const vector<bitset<REG_SIZE>>& registers, const string& str)
{
    int index = address_to_index(registers, str);
    return data[index].to_ullong();
}

// converts the address string into an index for data
int address_to_index(const vector<bitset<REG_SIZE>>& registers, const string& str)
{
    int open_par_pos = str.find('(');
    int close_par_pos = str.find(')');
    string op1 = str.substr(0, open_par_pos);
    string op2 = str.substr(open_par_pos + 1, close_par_pos - op1.length() - 1);

    int val1, val2;
    if (str[0] != 'R')
    {    // op1 is immediate, op2 is register
        val1 = stoi(op1, NULL, 10);
        val2 = registers[VALID_REGISTERS.at(op2)].to_ullong();
    }
    else
    {    // op1 is register, op2 is immediate
        val1 = registers[VALID_REGISTERS.at(op1)].to_ullong();
        val2 = stoi(op2, NULL, 10);
    }

    int address = val1 + val2;
    int index = (address - 256) / 4;
    return index;
}

// converts hex and decimal strings to decimal values
// use when executing instructions that have an immediate operand
int resolve_immediate(const string& str)
{
    int H_pos = str.find('H');
    if (H_pos != -1)
    {    // immediate is in hex
        string number_portion = str.substr(0, H_pos);
        return stoi(number_portion, NULL, 16);
    }
    else
    {    // number string is already in decimal
        return stoi(str, NULL, 10);
    }
}

// increments finish_log entry and pushed forward if flags.able_to_push is set
void attempt_push(vector<Instruction*>& active_instrs, const vector<Instruction>& running_instrs, FlagReg& flags, const Stage stage, const int cycle)
{
    Instruction cur_instr = *active_instrs[stage];

    if (not active_instrs[stage]->has_completed)
    {
        active_instrs[stage]->finish_log[stage] = cycle;
    }

    if (stage == MEM)
    {
        active_instrs[stage]->mem_count++;
    }

    update_flags(flags, active_instrs, running_instrs, stage);

    if (flags.able_to_push)
    {

        if (flags.finish_op_this_stage)
        {
            active_instrs[stage]->has_completed = true;
        }

        if (stage != WB)
        {
            active_instrs[stage + 1] = active_instrs[stage];
            active_instrs[stage + 1]->in_stage = stages[stage + 1];
            active_instrs[stage] = nullptr;
        }
        else
        {
            active_instrs[stage] = nullptr;
        }
    }
}

void update_data_hazards(vector<Instruction*>& active_instrs, const vector<Instruction>& running_instrs)
{
    // for each active instruction past IF
    for (int ii = NUM_STAGES - 1; ii >= 1; ii--)
    {

        //// compare cur_instr against new_instr
        // active instruction writes to a result regiser
        if (active_instrs[IF] && active_instrs[ii] && active_instrs[IF]->exists && active_instrs[ii]->exists && active_instrs[ii]->writes_to_register)
        {

            // result register matches one of the source registers in IF, (data hazard)
            if (active_instrs[ii]->result_reg == active_instrs[IF]->source_reg1 || active_instrs[ii]->result_reg == active_instrs[IF]->source_reg1)
            {

                // data hazard has already been cleared
                if (active_instrs[ii]->already_wrote_result)
                {
                    active_instrs[IF]->has_data_hazard = false;
                }
                // data hazard still exists
                else
                {
                    active_instrs[IF]->has_data_hazard = true;
                    active_instrs[IF]->nearest_data_hazard = active_instrs[ii]->instr_index;
                }
            }
        }
    }

    //// compare cur_instr against its nearest data hazard
    for (int ii = NUM_STAGES - 1; ii >= 0; ii--)
    {
        if (active_instrs[IF] && active_instrs[ii] && active_instrs[ii]->exists && active_instrs[ii]->has_data_hazard)
        {
            int hazard_index = active_instrs[IF]->instr_index - active_instrs[ii]->nearest_data_hazard;

            // cur_instr's data hazard already made it through pipeline
            if (hazard_index > NUM_STAGES - 1)
            {
                active_instrs[ii]->has_data_hazard = false;
            }

            // cur_instr's data hazard already wrote their result
            if (running_instrs[active_instrs[ii]->nearest_data_hazard].already_wrote_result)
            {
                active_instrs[ii]->has_data_hazard = false;
            }
        }
    }
}

void write_output(const vector<Instruction>& instruction_history, const string& output_file)
{
    ofstream file;
    file.open(output_file, ofstream::trunc);

    const Instruction* cur;
    int length;
    int pad_length;

    file << "Cycle Number for Each Stage        IF\tID\tEX3\tMEM\tWB" << endl;
    for (unsigned int ii = 0; ii < instruction_history.size(); ii++)
    {
        cur = &instruction_history[ii];
        file << cur->original_line;

        // pad everything to 35 spaces, then align with tabs
        length = cur->original_line.length();
        pad_length = NUM_PAD_SPACES - length;
        for (int jj = 0; jj < pad_length; jj++)
        {
            file << " ";
        }

        file << cur->finish_log[0] << "\t"
             << cur->finish_log[1] << "\t"
             << cur->finish_log[4] << "\t"
             << cur->finish_log[5] << "\t"
             << cur->finish_log[6] << endl;
    }

    file.close();
}

void print_output(const vector<Instruction>& instruction_history)
{
    const Instruction* cur;
    int length;
    int pad_length;

    cout << "Cycle Number for Each Stage        IF\tID\tEX3\tMEM\tWB" << endl;
    for (unsigned int ii = 0; ii < instruction_history.size(); ii++)
    {
        cur = &instruction_history[ii];
        cout << cur->original_line;

        // pad everything to 35 spaces, then align with tabs
        length = cur->original_line.length();
        pad_length = NUM_PAD_SPACES - length;
        for (int jj = 0; jj < pad_length; jj++)
        {
            cout << " ";
        }

        cout << cur->finish_log[0] << "\t"
             << cur->finish_log[1] << "\t"
             << cur->finish_log[4] << "\t"
             << cur->finish_log[5] << "\t"
             << cur->finish_log[6] << endl;
    }
}



// (for debugging)
// prints given instructions's member variables between braces to observe exact value
void print_instruction(const Instruction& instruction)
{
    cout << "----------------------------------------" << endl;
    cout << "-- PARSED VARIABLES --" << endl;
    cout << "line_number:{" << instruction.line_number << "}" << endl;
    cout << "original_line:{" << instruction.original_line << "}" << endl;
    cout << "label:{" << instruction.label << "}" << endl;
    cout << "opcode:{" << instruction.opcode << "}" << endl;
    cout << "arg1:{" << instruction.arg1 << "}" << endl;
    cout << "arg2:{" << instruction.arg2 << "}" << endl;
    cout << "arg3:{" << instruction.arg3 << "}" << endl;
    cout << "-- EXECUTION VARIABLES --" << endl;
    cout << "has_completed:{" << instruction.has_completed << "}" << endl;
    cout << "has_data_hazard:{" << instruction.has_data_hazard << "}" << endl;
    cout << "nearest_data_hazard:{" << instruction.nearest_data_hazard << "}" << endl;
    cout << "in_stage:{" << instruction.in_stage << "}" << endl;
    cout << "-- DOCUMENTATION VARIABLES --" << endl;
    cout << "finished_IF_at:{" << instruction.finish_log[0] << "}" << endl;
    cout << "finished_ID_at:{" << instruction.finish_log[1] << "}" << endl;
    cout << "finished_EX1_at:{" << instruction.finish_log[2] << "}" << endl;
    cout << "finished_EX2_at:{" << instruction.finish_log[3] << "}" << endl;
    cout << "finished_EX3_at:{" << instruction.finish_log[4] << "}" << endl;
    cout << "finished_MEM_at:{" << instruction.finish_log[5] << "}" << endl;
    cout << "finished_WB_at:{" << instruction.finish_log[6] << "}" << endl;
}

// (for debugging)
// prints every instruction's member variables between brackets
void print_instructions(const vector<Instruction>& instructions)
{
    for (unsigned int ii = 0; ii < instructions.size(); ii++)
    {
        print_instruction(instructions[ii]);
    }
}

// (for debugging)
// prints binary values from registers
void print_registers(const vector<bitset<REG_SIZE>>& registers)
{
    for (unsigned int ii = 0; ii < registers.size(); ii++)
    {
        cout << registers[ii].to_string() << " : " << registers[ii].to_ullong() << " : " << ii << endl;
    }
}

// (for debugging)
// prints the given string inside brackets to observe whitespace/emptiness
void examine_str(const string& name, const string& str)
{
    cout << name << ":{" << str << "}" << endl;
}

// (for debugging)
// prints the integer ascci value for each character in the string
void examine_str_ASCCI(const string& name, const string& str)
{
    cout << name << ":{";
    for (unsigned int ii = 0; ii < str.size() - 1; ii++)
    {
        cout << (int)str[ii] << ", ";
    }
    cout << (int)str[str.size() - 1] << "}" << endl;
}
