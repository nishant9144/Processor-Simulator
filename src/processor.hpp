#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "ds.hpp"
#include <string>
#include <fstream>
#include <vector>

class Processor {
protected:
    // Pipeline components
    instruction_memory instr_mem;
    register_memory reg_file;
    data_memory data_mem;
    program_counter pc;
    
    // Pipeline registers
    IF_ID_register_file IF_ID;
    ID_EX_register_file ID_EX;
    EX_MEM_register_file EX_MEM;
    MEM_WB_register_file MEM_WB;
    
    // Control signals
    ControlSignals control;
    
    // Cycle tracking
    int cycle_count = 0;
    
    // Instruction tracking for pipeline diagram
    std::vector<std::string> instruction_strings;
    std::vector<std::string> pipeline_states;
    
    // Load instructions from file
    void load_instructions(const std::string& filename);
    
    // Pipeline stage functions
    virtual void fetch();
    virtual void decode();
    virtual void execute();
    virtual void memory_access();
    virtual void write_back();
    
    // Generate pipeline diagram
    virtual void update_pipeline_diagram() = 0;
    
public:
    Processor() = default;
    virtual ~Processor() = default;
    
    void load_program(const std::string& filename);
    virtual void run_simulation(int max_cycles);
    void print_pipeline_diagram() const;
};

#endif // PROCESSOR_HPP