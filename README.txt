AUTHOR: Mason Herndon
CLASS: CMSC 411 - Computer Architecture
ASSIGNMENT: Term Project (MIPS interpreter)

DESCRIPTION:
    The executable "simulator" accepts three commandline filename arguments,
    loading a program of MIPS instructions from the first, data registers from
    the second, and outputing how many cycles each stage of each instruction
    took. Not to mention, it actually executes and stores the result in its
    internal registers and data file, allowing you to retriev your data with
    a simple modification to the source code.

INPUT:
    My "MIPS" interpreter not only accepts syntax outlined in the project
    documents, but also accepts input programs closer resembling real MIPS.

    Accepted:
        - Accepts input programs of any length, either running to completion,
            or until a user-defined limit is reached (in our case 100 cycles).
        - Input does not require "|" to seperate fields.
        - Spaces are asyntactic, having no affect on semantics as long as they
            do not appear inside an identifier.
        - Jumps and branches can accept line numbers instead of just labels.
        - Labels can appear on their own line, making it easier to define
            functions and code sections.
        - Lines can be left blank.
        - Data file has no size limit.

    Constraints:
        - The entire program is case insensitive, even labels.
        - Labels must not contain "(" or ")".
        - The given syntax for the project doesnt use "$" to dereference,
            meaning the expanded syntax can never fully work with real MIPS.
        - To use the variable sized input functionality, you must set
            ENABLE_UNLIMITED_INPUT to true, and increase the MAX_CYCLE_LIMIT to
            accomadate your larger sized program

ASSUMPTIONS:
    - Assuming that J (unconditional jump) terminates after the EX1 stage like
        like BEQ and BNE

KNOWN ISSUES:
    - I mistakenly thought that we were including stalled cycles in the total
        count for each stage. Because of this, portions of my output don't
        match the sample output. I considered the last stall of each stage,
        (meaning the stage to the left of the stalls in the pipeline table), to
        to be a part of the stage, and thus counted them. This makes logical
        sense because when considering structural hazards, a stalled stage
        still counts as an occupied stage.
    - I suspect there is an issue with my update_data_hazards() function when
        handling two data hazards at the same time. Logically, the "nearest"
        data hazard would outlive another, thus being the only one neccessary
        to keep track of, but I believe my has_data_hazard flag was being
        overwritten when the first hazard resolved.

HOW TO COMPILE:
    make

HOW TO RUN CODE:
    make run                (uses default input/output files)
    or
    ./simulator <instruction_file.txt> <data_file.txt> <output_file.txt>
