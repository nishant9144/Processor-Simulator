#include "processor.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <bitset>
#include "forward_processor.hpp"

void ForwardingProcessor::fetch()
{
    if(hazard_unit.flush){
        pc_handler.currPC -= 4;
    }
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
        IF_ID.instr_index = pc.instruction_address / 4;
    }
    else
    {
        if (IF_ID.instr_index != instr_mem.instructions.size() - 1 and !pc_handler.stall)
            IF_ID.instr_index++;
    }

    uint8_t rs1 = reg_file.r1 = (IF_ID.instruction >> 15) & 0x1F;
    uint8_t rs2 = reg_file.r2 = (IF_ID.instruction >> 20) & 0x1F;

    hazard_unit.if_id_ins = IF_ID.instruction;
    // In decode() function, after hazard detection
    hazard_unit.detect(ID_EX.IF_ID_Register_RD, EX_MEM.ID_EX_RegisterRD,
                       rs1, rs2, ID_EX.memRead, EX_MEM.memRead,
                       ID_EX.regWrite, EX_MEM.regWrite, MEM_WB.regWrite, MEM_WB.EX_MEM_RegisterRD);

    IF_ID.flush = hazard_unit.flush;
    pc_handler.branch_taken = hazard_unit.branch_taken;
    pc_handler.stall = hazard_unit.stall;
    if((ID_EX.instruction & 0x7F) == 0x67){
        pc_handler.branch_jump_PC = (ID_EX.tempr1_data + ID_EX.immediate) + 4 - pc_handler.currPC;
        pc_handler.branch_taken = true;
        pc_handler.stall = false;
    }else{
        pc_handler.branch_jump_PC = (ID_EX.immediate);
    }
}

void ForwardingProcessor::decode()
{
    bool flush = hazard_unit.flush;
    // Extract fields from instruction
    uint32_t opcode = (flush ? 0 : IF_ID.instruction & 0x7F);
    uint8_t rd = reg_file.rd = (flush ? 0 : (IF_ID.instruction >> 7) & 0x1F);
    uint8_t rs1 = reg_file.r1 = (flush ? 0 : (IF_ID.instruction >> 15) & 0x1F);
    uint8_t rs2 = reg_file.r2 = (flush ? 0 : (IF_ID.instruction >> 20) & 0x1F);
    uint32_t funct3 = (flush ? 0 : (IF_ID.instruction >> 12) & 0x7);
    uint32_t funct7 = (flush ? 0 : (IF_ID.instruction >> 25) & 0x7F);

    // Generate control signals
    generate_control_signals(hazard_unit.stall);

    // Update ID/EX register
    ID_EX.IF_ID_Register_RS1 = rs1;
    ID_EX.IF_ID_Register_RS2 = rs2;
    ID_EX.IF_ID_Register_RD = rd;

    bool jalrsig = (opcode == 0x67), jalsig = (opcode == 0x6F);
    // Update the ID/EX register
    reg_file.produce_read();
    if(jalrsig){
        ID_EX.tempr1_data = reg_file.r_data1;
    }

    if(opcode == 0x67 || opcode == 0x6F){
        ID_EX.reg1_data = IF_ID.program_counter;
        ID_EX.reg2_data = 4;
    }else{
        ID_EX.reg1_data = reg_file.r_data1;
        ID_EX.reg2_data = reg_file.r_data2;
    }


    /*  Here the immediate will be used for either the immediate addition in the ALU or the jumping                          */
    ID_EX.instruction = (flush ? 0 : IF_ID.instruction);

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
    if(opcode == 0x67 || opcode == 0x6F){
        hazard_unit.is_equal = false;
    }else{
        hazard_unit.is_equal    = reg_file.branch_eq;
    }

    if(hazard_unit.stall || flush){
        ID_EX.instr_index = SIZE_MAX;
    }
    else
        ID_EX.instr_index = IF_ID.instr_index;
}

void ForwardingProcessor::execute()
{
    forwarding_unit = ForwardingUnit();

    forwarding_unit.reg1_result = ID_EX.reg1_data;
    forwarding_unit.reg2_result = ID_EX.reg2_data;

    forwarding_unit.alu_result = EX_MEM.alu_result;
    forwarding_unit.wb_result = mux_wb.output;

    forwarding_unit.detect(EX_MEM.regWrite, EX_MEM.ID_EX_RegisterRD, MEM_WB.regWrite,
                           MEM_WB.EX_MEM_RegisterRD, ID_EX.IF_ID_Register_RS1,
                           ID_EX.IF_ID_Register_RS2);

    mux_alu = MUX_ALU();

    mux_alu.reg2_value = forwarding_unit.outputB;
    mux_alu.imm = ID_EX.immediate;

    mux_alu.handle();

    int64_t operand1 = forwarding_unit.outputA;
    int64_t operand2 = mux_alu.output;

    ALU::Operation op = ALU::Operation::ADD; // Default
    uint64_t opcode = ID_EX.instruction & 0x7F;
    if(opcode != 0x67 && opcode != 0x6F){
        generate_alu_ops(op);
    }

    // Perform ALU operation
    EX_MEM.alu_result = ALU::compute(operand1, operand2, op);

    // Forward data for memory operations (might need forwarding for store instructions)
    EX_MEM.write_data = forwarding_unit.outputB;

    // Forward control signals
    EX_MEM.regWrite = ID_EX.regWrite;
    EX_MEM.memToReg = ID_EX.memToReg;
    EX_MEM.memRead = ID_EX.memRead;
    EX_MEM.memWrite = ID_EX.memWrite;

    // Forward register destination
    EX_MEM.ID_EX_RegisterRD = ID_EX.IF_ID_Register_RD;
    EX_MEM.instr_index = ID_EX.instr_index;
}
