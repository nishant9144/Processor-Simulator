#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "ds.hpp"
#include <string>
#include <fstream>
#include <vector>

class Processor
{
protected:
    // Pipeline components
    instruction_memory instr_mem;
    register_memory reg_file;
    data_memory data_mem;
    program_counter pc;
    imm_gen im_gen;

    // Pipeline registers
    IF_ID_register_file IF_ID;
    ID_EX_register_file ID_EX;
    EX_MEM_register_file EX_MEM;
    MEM_WB_register_file MEM_WB;

    // Control signals
    ControlSignals control;
    PC_handler pc_handler;

    // Cycle tracking
    int cycle_count = 0;


    /*          For testing purpose                 */
    // Instruction tracking for pipeline diagram
    vector<string> instruction_strings;

    vector<string> pipeline_states;

    // Load instructions from file
    void load_instructions(const string &filename);

    // Pipeline stage functions
    void generate_control_signals(bool stall);
    void generate_alu_ops(ALU::Operation &operation);

    // Run in reverse order
    virtual void fetch() = 0;
    virtual void decode() = 0;
    virtual void execute() = 0;
    void memory_access();
    void write_back();

    // Generate pipeline diagram
    virtual void update_pipeline_diagram();

public:
    Processor() = default;
    virtual ~Processor() = default;

    void load_program(const string &filename);
    virtual void run_simulation(int max_cycles);
    void print_pipeline_diagram() const;
};

#endif // PROCESSOR_HPP