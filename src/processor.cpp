#include "processor.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <bitset>

void Processor::load_instructions(const string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }

    std::string line;
    uint32_t instruction = 0;
    while (std::getline(file, line))
    {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == '/')
        {
            continue;
        }

        std::stringstream ss(line);
        ss >> std::hex >> instruction;

        instr_mem.instructions.push_back(instruction);
        // Save the full assembly string (first column in diagram)
        instruction_strings.push_back(line);
        // Initialize the corresponding pipeline diagram row with the instruction string.
        pipeline_states.push_back(line);
    }

    file.close();
}


void Processor::load_program(const string &filename)
{
    // Reset processor state
    pc.instruction_address = 0;
    cycle_count = 0;

    // Clear pipeline registers
    IF_ID = IF_ID_register_file();
    ID_EX = ID_EX_register_file();
    EX_MEM = EX_MEM_register_file();
    MEM_WB = MEM_WB_register_file();

    // // Clear tracking data
    instruction_strings.clear();
    pipeline_states.clear();

    // Load instructions
    load_instructions(filename);
}

void Processor::generate_control_signals(bool stall)
{
    uint32_t opcode = IF_ID.instruction & 0x7F;
    uint32_t funct3 = (IF_ID.instruction >> 12) & 0x7;
    uint32_t funct7 = (IF_ID.instruction >> 25) & 0x7F;

    // Default control signals
    control = ControlSignals();

    if (!stall)
    {
        switch (opcode)
        {
        case 0x33: // R-type instructions -> 0110011
            control.regWrite = true;
            control.aluOp = 2; // R-type ALU operations
            break;

        case 0x13: // I-type ALU instructions -> 0010011
            control.regWrite = true;
            control.aluSrc = true;
            control.aluOp = 2; // I-type ALU operations
            break;

            // jalr is a I-type isntruction with different opcode

        case 0x03: // Load instructions -> 0000011
            control.memRead = true;
            control.regWrite = true;
            control.aluSrc = true;
            control.memToReg = true;
            break;

        case 0x23: // Store instructions -> 0100011
            control.memWrite = true;
            control.aluSrc = true;
            break;

        case 0x63: // Branch instructions -> 1100111
            control.branch = true;
            control.aluOp = 1; // Branch comparison
            break;

        default:
            // Unknown opcode - NOP
            break;
        }
    }
}

void Processor::generate_alu_ops(ALU::Operation &operation)
{
    uint32_t funct3 = (ID_EX.instruction >> 12) & 0x7;
    uint32_t funct7 = (ID_EX.instruction >> 25) & 0x7F;
    uint8_t aluOp = ID_EX.aluOp;

    // Based on aluOp
    if (aluOp == 0)
    {
        // Load/Store: always ADD
        operation = ALU::Operation::ADD;
    }
    else if (aluOp == 1)
    {
        // Branch: always SUB
        operation = ALU::Operation::SUB;
    }
    else if (aluOp == 2)
    {
        // R-type or I-type
        switch (funct3)
        {
        case 0:
            if (funct7 == 0x20)
            {
                operation = ALU::Operation::SUB;
            }
            else
            {
                operation = ALU::Operation::ADD;
            }
            break;
        case 1:
            operation = ALU::Operation::SLL;
            break;

        /*      Unknown         */
        case 2:
            operation = ALU::Operation::SLT;
            break;
        case 3:
            operation = ALU::Operation::SLTU;
            // break;

        case 4:
            operation = ALU::Operation::XOR;
            break;
        case 5:
            if (funct7 == 0x20)
            {
                operation = ALU::Operation::SRA;
            }
            else
            {
                operation = ALU::Operation::SRL;
            }
            break;
        case 6:
            operation = ALU::Operation::OR;
            break;
        case 7:
            operation = ALU::Operation::AND;
            break;
        }
    }
}

void Processor::memory_access()
{
    data_mem.addr = EX_MEM.alu_result;
    data_mem.w_data = EX_MEM.write_data;

    data_mem.memRead = EX_MEM.memRead;
    data_mem.memWrite = EX_MEM.memWrite;

    // Access memory if needed

    data_mem.read();
    MEM_WB.read_data = data_mem.r_data;

    // if (EX_MEM.memWrite)

    data_mem.write();

    // Forward ALU result
    MEM_WB.alu_result = EX_MEM.alu_result;

    // Forward control signals
    MEM_WB.regWrite = EX_MEM.regWrite; // regWrite
    MEM_WB.memToReg = EX_MEM.memToReg; // memToReg

    // Forward register destination
    MEM_WB.EX_MEM_RegisterRD = EX_MEM.ID_EX_RegisterRD;

    MEM_WB.instr_index = EX_MEM.instr_index;
}

void Processor::write_back()
{
    // Write to register file if needed
    reg_file.regWrite = MEM_WB.regWrite;

    mux_wb = MUX_WB();

    mux_wb.mem_to_reg = MEM_WB.memToReg;
    mux_wb.mem_value = MEM_WB.read_data;
    mux_wb.alu_value = MEM_WB.alu_result;

    mux_wb.handle();
    
    reg_file.w_data = mux_wb.output;
    reg_file.rd = MEM_WB.EX_MEM_RegisterRD;
    reg_file.write();
    
    data_mem.wb_index = MEM_WB.instr_index;
}

// I have not made use of the MUX_WB here.

void Processor::update_pipeline_diagram()
{
    // For each instruction row, we will append (using semicolon delimiter)
    // the stage if that instruction is in that stage in the current cycle.
    // For simplicity we assume that your pipeline registers have been modified to include
    // an "instr_index" field that tells you which fetched instruction is currently in that stage.
    // For example, assume:
    //    IF_ID.instr_index, ID_EX.instr_index, EX_MEM.instr_index, MEM_WB.instr_index
    // are set to the index in instruction_strings (and pipeline_diagram).
    // If a pipeline register does not hold an instruction, we consider its value to be, say, SIZE_MAX.

    // For each fetched instruction, decide which stage (if any) it occupies this cycle.
    // The assignment sample output shows that if an instruction has not yet entered a stage, then
    // an empty field (or a space) is printed; if it is not in the pipeline at all (already finished)
    // then a dash (-) is printed.
    // Here is one way to do it:

    // Create a temporary vector for the stage labels of this cycle (one per instruction).
    vector<string> cycleStages(pipeline_states.size(), ""); // initially empty

    // Check IF stage
    if (IF_ID.instr_index != SIZE_MAX)
        cycleStages[IF_ID.instr_index] = "IF ";
    // Check ID stage
    if (ID_EX.instruction != 0 && ID_EX.instr_index != SIZE_MAX)
        cycleStages[ID_EX.instr_index] = "ID ";
    // Check EX stage
    if (EX_MEM.instr_index != SIZE_MAX)
        cycleStages[EX_MEM.instr_index] = "EX ";
    // Check MEM stage
    if (MEM_WB.instr_index != SIZE_MAX)
        cycleStages[MEM_WB.instr_index] = "MEM";
    // For WB stage, assume that you have a flag (or WB register) that indicates which instruction is being written back.
    // For example, let wb_index be a member that you update in write_back().
    if (data_mem.wb_index != SIZE_MAX)
        cycleStages[data_mem.wb_index] = "WB ";
    
    // Now update each row: for each instruction that has been fetched,
    // append a semicolon and the stage label for this cycle.
    // If the instruction was not in any stage in this cycle, we append either an empty field or a dash,
    // following the assignment sample (we use a dash for instructions that have finished).
    for (size_t i = 0; i < pipeline_states.size(); i++)
    {
        // Decide what to append:
        // If the instruction has not been fetched yet, you may leave it unchanged.
        // If it has been fetched but is not in any stage in this cycle, then:
        //   – if i > current_fetch_index (i.e. not yet fetched) append an empty field;
        //   – otherwise, if it has finished, append a dash.
        string stage;
        if (cycleStages[i] != "")
        {
            stage = cycleStages[i];
        }
        else
        {
            // For this example, if the instruction index is less than current_fetch_index,
            // assume it has finished execution.
            if (i < IF_ID.instr_index)
                stage = " - ";
            else
                stage = "   ";
        }
        // Append the field using semicolon as delimiter.
        pipeline_states[i] += ";" + stage;
    }
}


void Processor::print_pipeline_diagram() const
{
    for (const auto &state : pipeline_states)
    {
        std::cout << state << std::endl;
    }

    // Print final register state
    std::cout << "\nFinal Register Values:\n";
    for (int i = 0; i < 32; i++)
    {
        if (reg_file.registers[i] != 0)
        {
            std::cout << "x" << i << ": " << reg_file.registers[i] << std::endl;
        }
    }

    // Print memory state
    std::cout << "\nFinal Memory Values:\n";
    for (const auto &pair : data_mem.data_memory)
    {
        std::cout << "0x" << std::hex << pair.first << ": 0x" << pair.second << std::dec << std::endl;
    }

    std::cout << "\nTotal cycles: " << cycle_count << std::endl;
}

void Processor::run_simulation(int max_cycles)
{
    for (int i = 0; i < max_cycles; i++)
    {
        // Exit if we've processed all instructions and the pipeline is empty
        if (pc.instruction_address >= instr_mem.instructions.size() &&
            IF_ID.instr_index == SIZE_MAX && ID_EX.instr_index == SIZE_MAX && 
            EX_MEM.instr_index == SIZE_MAX && MEM_WB.instr_index == SIZE_MAX)
        {
            break;
        }

        // Execute pipeline stages in reverse order (to avoid data overwriting)
        write_back();
        memory_access();
        execute();
        decode();
        fetch();
        

        // Update cycle count and pipeline diagram
        cycle_count++;
        update_pipeline_diagram();
    }
}

/*
00a282b3 add x5 x5 x10;IF;ID;EX;MEM;WB;-;-;-;-;-;-;-;-;-;-
00a28133 add x2 x5 x10; ;IF;IF;IF;ID;EX;MEM;WB;-;-;-;-;-;-;-
00a281b3 add x3 x5 x10; ; ; ; ;IF;ID;EX;MEM;WB;-;-;-;-;-;-
00a28233 add x4 x5 x10; ; ; ; ; ;IF;ID;EX;MEM;WB;-;-;-;-;-
00a284b3 add x9 x5 x10; ; ; ; ; ; ;IF;ID;EX;MEM;WB;-;-;-;-
00a28333 add x6 x5 x10; ; ; ; ; ; ; ;IF;ID;EX;MEM;WB;-;-;-
00a283b3 add x7 x5 x10; ; ; ; ; ; ; ; ;IF;ID;EX;MEM;WB;-;-
00a28433 add x8 x5 x10; ; ; ; ; ; ; ; ; ;IF;ID;EX;MEM;WB;-
*/