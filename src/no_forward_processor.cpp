#include "processor.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <bitset>
#include "no_forward_processor.hpp"

void NoForwardingProcessor::fetch()
{
    pc_handler.handle();
    pc.instruction_address = pc_handler.currPC;
    // Check if we've reached the end of the instruction memory
    if (pc.instruction_address / 4 >= instr_mem.instructions.size())
    {
        // cout << "Processing Done" << endl;
        IF_ID.instr_index = SIZE_MAX;
        IF_ID.instruction = 0; // Clear the instruction to indicate no more instructions
        return;
    }
    instr_mem.address = pc.instruction_address;
    instr_mem.fetch();
    IF_ID.instruction = instr_mem.instruction;
    IF_ID.program_counter = pc.instruction_address;

    if (IF_ID.flush)
    {
        IF_ID.instr_index = pc.instruction_address/4;
    }else{
        if(IF_ID.instr_index == instr_mem.instructions.size()-1){
            // IF_ID.instr_index = SIZE_MAX;
        }
        else
        {
            if (!pc_handler.stall)
                IF_ID.instr_index++;
        }
    }

    uint8_t rs1 = reg_file.r1 = (IF_ID.instruction >> 15) & 0x1F;
    uint8_t rs2 = reg_file.r2 = (IF_ID.instruction >> 20) & 0x1F;

    // In decode() function, after hazard detection
    hazard_unit.detect(ID_EX.IF_ID_Register_RD, EX_MEM.ID_EX_RegisterRD,
                       rs1, rs2, ID_EX.memRead, EX_MEM.memRead,
                       ID_EX.regWrite, EX_MEM.regWrite);
    IF_ID.flush = hazard_unit.flush;
    pc_handler.branch_taken = hazard_unit.branch_taken;
    pc_handler.stall = hazard_unit.stall;
    pc_handler.branch_jump_PC = ID_EX.immediate;
    if(hazard_unit.flush){
        pc_handler.branch_jump_PC -= 4;
    }
}

void NoForwardingProcessor::decode()
{
    bool flush = hazard_unit.flush;
    // Extract fields from instruction
    uint32_t opcode = (flush ? 0 : IF_ID.instruction & 0x7F);
    uint8_t rd = reg_file.rd = (flush ? 0 : (IF_ID.instruction >> 7) & 0x1F);
    uint8_t rs1 = reg_file.r1 = (flush ? 0 :  (IF_ID.instruction >> 15) & 0x1F);
    uint8_t rs2 = reg_file.r2 = (flush ? 0 :  (IF_ID.instruction >> 20) & 0x1F);
    uint32_t funct3 = (flush ? 0 :  (IF_ID.instruction >> 12) & 0x7);
    uint32_t funct7 = (flush ? 0 :  (IF_ID.instruction >> 25) & 0x7F);

    // Generate control signals
    generate_control_signals(hazard_unit.stall);

    // Update ID/EX register
    ID_EX.IF_ID_Register_RS1 = rs1;
    ID_EX.IF_ID_Register_RS2 = rs2;
    ID_EX.IF_ID_Register_RD = rd;

    // Update the ID/EX register
    reg_file.produce_read();
    ID_EX.reg1_data = reg_file.r_data1;
    ID_EX.reg2_data = reg_file.r_data2;

    ID_EX.instruction = (flush ? 0 : IF_ID.instruction);

    /*  Here the immediate will be used for either the immediate addition in the ALU or the jumping                          */
        // Generate immediate based on instruction type
    im_gen.instruction = IF_ID.instruction;
    im_gen.generate();
    ID_EX.immediate = im_gen.extended;

    // Update control signals
    ID_EX.regWrite = control.regWrite;
    ID_EX.memToReg = control.memToReg;
    ID_EX.memRead = control.memRead;
    ID_EX.memWrite = control.memWrite;
    ID_EX.aluSrc = control.aluSrc;
    ID_EX.aluOp = control.aluOp;

    hazard_unit.instruction = IF_ID.instruction;
    hazard_unit.is_equal = reg_file.branch_eq;
    /*          HERE I NEED TO UPDATE THE BRANCH_JUMP_ADDRESS IN THE PC_HANDLER         */

    

    if(hazard_unit.stall || flush){
        ID_EX.instr_index = SIZE_MAX;
    }
    else
        ID_EX.instr_index = IF_ID.instr_index;
}

void NoForwardingProcessor::execute()
{
    // Choose operands
    int64_t operand1 = ID_EX.reg1_data;
    int64_t operand2 = ID_EX.aluSrc ? ID_EX.immediate : ID_EX.reg2_data;

    /*      Generate the operation        */

    // Perform ALU operation
    ALU::Operation op = ALU::Operation::ADD; // Default
    // Calculate ALU operation
    generate_alu_ops(op);

    EX_MEM.alu_result = ALU::compute(operand1, operand2, op);

    // Forward data for memory operations
    EX_MEM.write_data = ID_EX.reg2_data;

    // Forward control signals
    EX_MEM.regWrite = ID_EX.regWrite; // regWrite
    EX_MEM.memToReg = ID_EX.memToReg; // memToReg
    EX_MEM.memRead = ID_EX.memRead;   // memRead
    EX_MEM.memWrite = ID_EX.memWrite; // memWrite

    // Forward register destination
    EX_MEM.ID_EX_RegisterRD = ID_EX.IF_ID_Register_RD;
    EX_MEM.instr_index = ID_EX.instr_index;
}