#include <iostream>
#include <string>
#include "memory.hpp"
#include "utils.hpp"
using std::string;

extern const bool ENABLE_UNLIMITED_INPUT = false; // true enables variable sized input
extern const int OPTIONAL_CYCLE_LIMIT = 100; // custom cycle limit
extern const int MAX_CYCLE_LIMIT = 1000; // amount of instruction memory given to program

const string DEFAULT_INST_FILE = "default_inst.txt";
const string DEFAULT_DATA_FILE = "default_data_segment.txt";
const string DEFAULT_OUTPUT_FILE = "default_output.txt";
const string CUSTOM_INST_FILE = "custom_inst.txt";
const int EXPECTED_NUM_ARGS = 4;

int main(const int argc, const char* argv[]){

    string instruction_file;
    string data_file;
    string output_file;

    // assign default/specified filenames
    if (argc < EXPECTED_NUM_ARGS){

        // use defaults filenames
        instruction_file = DEFAULT_INST_FILE;
        data_file = DEFAULT_DATA_FILE;
        output_file = DEFAULT_OUTPUT_FILE;
    }
    else {

        // use command line arguments
        instruction_file = argv[1];
        data_file = argv[2];
        output_file = argv[3];
    }

    Memory memory;
    load_program(memory.program, instruction_file);
    load_data(memory.data, data_file);
    run_program(memory, output_file);

    return 0;
}
