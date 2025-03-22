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
        std::cerr << "Error: Could not open file " << filename << std::endl;
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

        // Parse binary instruction
        std::stringstream ss(line);
        ss >> std::hex >> instruction;

        // Store instruction
        instr_mem.instructions.push_back(instruction);

        /*          For testing purpose                 */
        // Store string representation for pipeline diagram
        instruction_strings.push_back(line);
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

    // Clear tracking data
    instruction_strings.clear();
    pipeline_states.clear();

    // Load instructions
    load_instructions(filename);
}

void Processor::fetch()
{
    // Yaha ek condition for if the line of instructions get over

    if(!pc.stall)
    {
        IF_ID.instruction = instr_mem.instructions[pc.instruction_address / 4];
        IF_ID.program_counter = pc.instruction_address;
    }
    if(IF_ID.flush)
    {
        IF_ID.instruction = 0;
        IF_ID.program_counter = 0;
    }

}


void Processor::generate_control_signals()
{
    uint32_t opcode = IF_ID.instruction & 0x7F;
    uint32_t funct3 = (IF_ID.instruction >> 12) & 0x7;
    uint32_t funct7 = (IF_ID.instruction >> 25) & 0x7F;

    // Default control signals
    control = ControlSignals();

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

        case 0x67: // Branch instructions -> 1100111
            control.branch = true;
            control.aluOp = 1; // Branch comparison
            break;

        default:
            // Unknown opcode - NOP
            break;
    }
}

void Processor::decode()
{
    // Extract fields from instruction
    uint32_t opcode = IF_ID.instruction & 0x7F;
    uint8_t rd = reg_file.rd = (IF_ID.instruction >> 7) & 0x1F;
    uint8_t rs1 = reg_file.r1 = (IF_ID.instruction >> 15) & 0x1F;
    uint8_t rs2 = reg_file.r2 = (IF_ID.instruction >> 20) & 0x1F;
    uint32_t funct3 = (IF_ID.instruction >> 12) & 0x7;
    uint32_t funct7 = (IF_ID.instruction >> 25) & 0x7F;

    // Generate immediate based on instruction type
    int64_t imm = 0;

    // I-type immediate
    if (opcode == 0x13 || opcode == 0x03)
    {
        imm = (IF_ID.instruction >> 20) & 0xFFF;
        // Sign extend
        if (imm & 0x800)
            imm |= 0xFFFFFFFFFFFFF000;
    }
    // S-type immediate
    else if (opcode == 0x23)
    {
        imm = ((IF_ID.instruction >> 7) & 0x1F) | ((IF_ID.instruction >> 25) & 0xFE0);
        // Sign extend
        if (imm & 0x800)
            imm |= 0xFFFFFFFFFFFFF000;
    }
    // B-type immediate
    else if (opcode == 0x63)
    {
        imm = ((IF_ID.instruction >> 8) & 0xF) | ((IF_ID.instruction >> 25) & 0x3F0) |
              ((IF_ID.instruction << 4) & 0x800) | ((IF_ID.instruction >> 19) & 0x1000);
        // Sign extend
        if (imm & 0x1000)
            imm |= 0xFFFFFFFFFFFFE000;
    }

    // Generate control signals
    generate_control_signals();

    // Update ID/EX register
    ID_EX.IF_ID_Register_RS1 = rs1;
    ID_EX.IF_ID_Register_RS2 = rs2;
    ID_EX.IF_ID_Register_RD = rd;

    // Update the ID/EX register
    reg_file.produce_read();
    ID_EX.readData1 = reg_file.r_data1;
    ID_EX.readData2 = reg_file.r_data2;

    ID_EX.instruction = IF_ID.instruction;

    /*  Here the immediate will be used for either the immediate addition in the ALU or the jumping                          */
    ID_EX.immediate = imm;

    // Update control signals
    ID_EX.regWrite = control.regWrite;
    ID_EX.memToReg = control.memToReg;
    ID_EX.memRead = control.memRead;
    ID_EX.memWrite = control.memWrite;
    ID_EX.branch = control.branch;
    ID_EX.aluSrc = control.aluSrc;
    ID_EX.aluOp = control.aluOp;

    // Check for hazards
    hazard_unit.detect(ID_EX.memRead, ID_EX.IF_ID_Register_RD, rs1, rs2);
}

void Processor::generate_alu_ops(ALU::Operation& operation)
{
    uint32_t funct3 = (ID_EX.instruction >> 12) & 0x7;
    uint32_t funct7 = (ID_EX.instruction >> 25) & 0x7F;
    uint8_t aluOp   = ID_EX.aluOp;

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
            break;

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

void Processor::execute()
{
    // Choose operands
    int64_t operand1 = ID_EX.readData1;
    int64_t operand2 = ID_EX.aluSrc ? ID_EX.immediate : ID_EX.readData2;


    /*      Generate the operation        */
    // Perform ALU operation
    ALU::Operation op = ALU::Operation::ADD; // Default
    // Calculate ALU operation
    generate_alu_ops(op);

    EX_MEM.alu_result = ALU::compute(operand1, operand2, op);

    // Forward data for memory operations
    EX_MEM.write_data = ID_EX.readData2;

    // Forward control signals
    EX_MEM.regWrite = ID_EX.regWrite; // regWrite
    EX_MEM.memToReg = ID_EX.memToReg; // memToReg
    EX_MEM.memRead  = ID_EX.memRead;   // memRead
    EX_MEM.memWrite = ID_EX.memWrite; // memWrite
    EX_MEM.branch   = ID_EX.branch;     // branch

    // Forward register destination
    EX_MEM.ID_EX_RegisterRD = ID_EX.IF_ID_Register_RD;
}

void Processor::memory_access()
{
    data_mem.addr   = EX_MEM.alu_result;
    data_mem.w_data = EX_MEM.write_data;


    // Access memory if needed
    if (EX_MEM.memRead)
    {
        data_mem.read();
        MEM_WB.read_data = data_mem.r_data;
    }

    if (EX_MEM.memWrite)
    { 
        data_mem.write();
    }

    // Forward ALU result
    MEM_WB.alu_result = EX_MEM.alu_result;

    // Forward control signals
    MEM_WB.regWrite = EX_MEM.regWrite; // regWrite
    MEM_WB.memToReg = EX_MEM.memToReg; // memToReg

    // Forward register destination
    MEM_WB.EX_MEM_RegisterRD = EX_MEM.ID_EX_RegisterRD;
}

void Processor::write_back()
{
    // Write to register file if needed
    if ((reg_file.regWrite = MEM_WB.regWrite) = true)
    { 
        int64_t write_data = MEM_WB.memToReg ? MEM_WB.read_data : MEM_WB.alu_result;
        reg_file.w_data = write_data;
        reg_file.rd = MEM_WB.EX_MEM_RegisterRD;
        reg_file.write();
    }
}

void Processor::update_pipeline_diagram()
{
    std::stringstream ss;

    ss << "Cycle " << cycle_count << ": ";

    // Track which instructions are in which pipeline stage
    if (IF_ID.instruction != 0)
    {
        ss << "IF: " << IF_ID.program_counter << " ";
    }
    else
    {
        ss << "IF: - ";
    }

    if (ID_EX.IF_ID_Register_RD != 0)
    {
        ss << "ID: " << ID_EX.IF_ID_Register_RD << " ";
    }
    else
    {
        ss << "ID: - ";
    }

    if (EX_MEM.ID_EX_RegisterRD != 0)
    {        reg_file.regWrite = true;

        ss << "EX: " << EX_MEM.ID_EX_RegisterRD << " ";
    }
    else
    {
        ss << "EX: - ";
    }

    if (MEM_WB.EX_MEM_RegisterRD != 0)
    {
        ss << "MEM: " << MEM_WB.EX_MEM_RegisterRD << " ";
    }
    else
    {
        ss << "MEM: - ";
    }

    if (MEM_WB.regWrite)
    {
        ss << "WB: " << MEM_WB.EX_MEM_RegisterRD;
    }
    else
    {
        ss << "WB: -";
    }

    pipeline_states.push_back(ss.str());
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
            IF_ID.instruction == 0 &&
            ID_EX.IF_ID_Register_RD == 0 &&
            EX_MEM.ID_EX_RegisterRD == 0 &&
            MEM_WB.EX_MEM_RegisterRD == 0)
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