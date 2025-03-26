#include "processor.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <bitset>
#include "no_forward_processor.hpp"

void NoForwardingProcessor::decode()
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
        if (imm & 0x1000)
            imm |= 0xFFFFFFFFFFFFE000;
    }

    bool stall = hazard_unit.stall;
    // Generate control signals
    generate_control_signals(stall);

    // Update ID/EX register
    ID_EX.IF_ID_Register_RS1 = rs1;
    ID_EX.IF_ID_Register_RS2 = rs2;
    ID_EX.IF_ID_Register_RD = rd;

    // Update the ID/EX register
    reg_file.produce_read();
    ID_EX.reg1_data = reg_file.r_data1;
    ID_EX.reg2_data = reg_file.r_data2;

    ID_EX.instruction = IF_ID.instruction;

    /*  Here the immediate will be used for either the immediate addition in the ALU or the jumping                          */
    ID_EX.immediate = imm;

    // Update control signals
    ID_EX.regWrite = control.regWrite;
    ID_EX.memToReg = control.memToReg;
    ID_EX.memRead = control.memRead;
    ID_EX.memWrite = control.memWrite;
    ID_EX.aluSrc = control.aluSrc;
    ID_EX.aluOp = control.aluOp;

    hazard_unit.instruction = IF_ID.instruction;
    hazard_unit.is_equal = reg_file.branch_eq;

    // In decode() function, after hazard detection
    hazard_unit.detect(rd, EX_MEM.ID_EX_RegisterRD,
                       rs1, rs2, ID_EX.memRead, EX_MEM.memRead,
                       ID_EX.regWrite, EX_MEM.regWrite);

    IF_ID.flush = hazard_unit.flush;

    pc_handler.branch_taken = hazard_unit.branch_taken;
    pc_handler.stall = hazard_unit.stall;

    /*          HERE I NEED TO UPDATE THE BRANCH_JUMP_ADDRESS IN THE PC_HANDLER         */

    pc_handler.branch_jump_PC = IF_ID.program_counter + ID_EX.immediate;

    // Update PC handler based on hazard detection
    // pc_handler.stall = hazard_unit.stall;

    // // For branch handling in ID stage (for no-forwarding processor)
    // if (opcode == 0x63 && !hazard_unit.stall)
    // { // Branch instructions (BEQ, BNE, etc.)
    //     // Set instruction for hazard unit
    //     hazard_unit.instruction = IF_ID.instruction;

    //     // Check branch condition - branch resolution in ID stage
    //     bool branch_taken = false;
    //     switch (funct3)
    //     {
    //     case 0:                                        // BEQ
    //         hazard_unit.is_equal = reg_file.branch_eq; // Using the branch_eq flag we already have
    //         branch_taken = reg_file.branch_eq;
    //         break;
    //     case 1: // BNE
    //         branch_taken = !reg_file.branch_eq;
    //         break;
    //         // Add other branch types as needed
    //     }

    //     if (branch_taken)
    //     {
    //         // Calculate branch target (using immediate we already calculated)
    //         // For B-type instructions we need to recalculate the immediate correctly
    //         imm = 0;
    //         imm = ((int32_t)(IF_ID.instruction & 0x80000000)) >> 19;
    //         imm |= ((IF_ID.instruction & 0x7E000000) >> 20);
    //         imm |= ((IF_ID.instruction & 0x00000F00) >> 7);
    //         imm |= ((IF_ID.instruction & 0x00000080) << 4);
    //         if (imm & 0x1000)
    //             imm |= 0xFFFFFFFFFFFFE000; // Sign extend

    //         // Set PC handler for branch
    //         pc_handler.branch_taken = true;
    //         pc_handler.branch_jump_PC = imm;

    //         // Flush the pipeline
    //         IF_ID.flush = true;
    //     }
    //     else
    //     {
    //         // Branch not taken, continue normal execution
    //         pc_handler.branch_taken = false;
    //     }
    // }
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
}