#include "../src/processor.hpp"
#include <cassert>
#include <iostream>
#include <string>

// Test class that exposes protected members for testing
class TestProcessor : public Processor {
public:
    // Make protected methods accessible
    using Processor::generate_control_signals;
    
    // Allow direct manipulation of IF_ID register
    void set_instruction(uint32_t instr) {
        IF_ID.instruction = instr;
    }
    
    // Allow access to control signals for verification
    ControlSignals get_control_signals() const {
        return control;
    }
};

int main() {
    TestProcessor processor;
    ControlSignals signals;
    
    std::cout << "Testing control signal generation for different instruction types..." << std::endl;
    
    // Test R-type instruction (ADD: 0x33 opcode)
    uint32_t r_type_instr = 0x00a08433; // add x8, x1, x10
    processor.set_instruction(r_type_instr);
    processor.generate_control_signals();
    signals = processor.get_control_signals();
    
    assert(signals.regWrite == true);
    assert(signals.aluOp == 2);
    assert(signals.memRead == false);
    assert(signals.memWrite == false);
    assert(signals.aluSrc == false);
    assert(signals.memToReg == false);
    assert(signals.branch == false);
    std::cout << "R-type instruction control signals: PASSED" << std::endl;
    
    // Test I-type ALU instruction (ADDI: 0x13 opcode)
    uint32_t i_type_instr = 0x00500293; // addi x5, x0, 5
    processor.set_instruction(i_type_instr);
    processor.generate_control_signals();
    signals = processor.get_control_signals();
    
    assert(signals.regWrite == true);
    assert(signals.aluOp == 2);
    assert(signals.aluSrc == true);
    assert(signals.memRead == false);
    assert(signals.memWrite == false);
    assert(signals.memToReg == false);
    assert(signals.branch == false);
    std::cout << "I-type ALU instruction control signals: PASSED" << std::endl;
    
    // Test Load instruction (LW: 0x03 opcode)
    uint32_t load_instr = 0x0040a503; // lw x10, 4(x1)
    processor.set_instruction(load_instr);
    processor.generate_control_signals();
    signals = processor.get_control_signals();
    
    assert(signals.memRead == true);
    assert(signals.regWrite == true);
    assert(signals.aluSrc == true);
    assert(signals.memToReg == true);
    assert(signals.memWrite == false);
    assert(signals.branch == false);
    std::cout << "Load instruction control signals: PASSED" << std::endl;
    
    // Test Store instruction (SW: 0x23 opcode)
    uint32_t store_instr = 0x00a4a023; // sw x10, 0(x9)
    processor.set_instruction(store_instr);
    processor.generate_control_signals();
    signals = processor.get_control_signals();
    
    assert(signals.memWrite == true);
    assert(signals.aluSrc == true);
    assert(signals.regWrite == false);
    assert(signals.memRead == false);
    assert(signals.memToReg == false);
    assert(signals.branch == false);
    std::cout << "Store instruction control signals: PASSED" << std::endl;
    
    // Test Branch instruction (BEQ: 0x63 opcode)
    uint32_t branch_instr = 0x00b50463; // beq x10, x11, 8
    processor.set_instruction(branch_instr);
    processor.generate_control_signals();
    signals = processor.get_control_signals();
    
    assert(signals.branch == true);
    assert(signals.aluOp == 1);
    assert(signals.regWrite == false);
    assert(signals.memRead == false);
    assert(signals.memWrite == false);
    assert(signals.aluSrc == false);
    assert(signals.memToReg == false);
    std::cout << "Branch instruction control signals: PASSED" << std::endl;
    
    // Test unknown opcode (should default to NOP with all signals false)
    uint32_t unknown_instr = 0xFFFFFFFF; // Invalid instruction
    processor.set_instruction(unknown_instr);
    processor.generate_control_signals();
    signals = processor.get_control_signals();
    
    assert(signals.regWrite == false);
    assert(signals.memRead == false);
    assert(signals.memWrite == false);
    assert(signals.aluSrc == false);
    assert(signals.memToReg == false);
    assert(signals.branch == false);
    assert(signals.aluOp == 0);
    std::cout << "Unknown opcode control signals: PASSED" << std::endl;
    
    std::cout << "All tests PASSED!" << std::endl;
    return 0;
}