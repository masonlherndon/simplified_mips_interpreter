#ifndef FLAGS_HPP
#define FLAGS_HPP

#include <vector>
#include "instruction.hpp"
#include "stage.hpp"

struct FlagReg {

    // stage flags
    bool has_structural_hazard = false;
    bool has_data_hazard = false;
    bool able_to_push = false;
    bool finish_op_this_stage = false;
    bool is_stalling = false;

    // program flags
    bool control_hazard_exists = false;
    bool able_to_insert = true;
    bool finishing_up = false; // constant after set
    bool program_complete = false; // constant after set
};

#endif
