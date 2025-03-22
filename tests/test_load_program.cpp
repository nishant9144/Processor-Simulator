#include "../src/processor.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

// Test class that exposes protected members for testing
class TestProcessor : public Processor {
public:
    // Access to protected members for verification
    bool verify_reset_state() const {
        // Check PC reset
        if (pc.instruction_address != 0) return false;
        
        // Check cycle count reset
        if (cycle_count != 0) return false;
        
        // Check pipeline registers cleared
        if (IF_ID.instruction != 0 || IF_ID.program_counter != 0 || IF_ID.flush != false) return false;
        
        // Verify ID_EX cleared (sample verification - expand as needed)
        if (ID_EX.IF_ID_Register_RD != 0 || ID_EX.readData1 != 0) return false;
        
        // Verify EX_MEM cleared
        if (EX_MEM.ID_EX_RegisterRD != 0) return false;
        
        // Verify MEM_WB cleared
        if (MEM_WB.EX_MEM_RegisterRD != 0) return false;
        
        // Verify tracking structures cleared
        if (!instruction_strings.empty() || !pipeline_states.empty()) return false;
        
        return true;
    }
    
    bool verify_instructions_loaded() const {
        // Verify instructions were loaded
        return !instr_mem.instructions.empty();
    }
    
    size_t get_instruction_count() const {
        return instr_mem.instructions.size();
    }
    
    uint32_t get_instruction(size_t index) const {
        if (index < instr_mem.instructions.size()) {
            return instr_mem.instructions[index];
        }
        return 0;
    }
    
    // Make protected methods accessible
    using Processor::load_program;
};

// Create a test file with sample instructions
void create_test_file(const std::string &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to create test file" << std::endl;
        exit(1);
    }
    
    // Add some test instructions with comments
    file << "# Test instruction file\n";
    file << "00500293  # addi x5, x0, 5\n";
    file << "00a08433  # add x8, x1, x10\n";
    file << "\n";  // Empty line
    file << "// Comment in a different style\n";
    file << "00044503  # lbu x10, 0(x8)\n";
    
    file.close();
}

int main() {
    // Create test file
    std::string test_filename = "test_program.txt";
    create_test_file(test_filename);
    
    // Create processor and test initial state
    TestProcessor processor;
    
    // Add some fake data to verify it gets cleared
    processor.load_program(test_filename); // First load to populate
    
    // Modify pipeline to verify reset
    // We can't directly modify protected members, but we can load a second file
    // to verify the reset occurs
    
    // Load the program (which should reset everything)
    processor.load_program(test_filename);
    
    // Verify state was reset
    bool reset_ok = processor.verify_reset_state();
    if (!reset_ok) {
        std::cerr << "Failed: Processor state was not properly reset" << std::endl;
        return 1;
    } else {
        std::cout << "Passed: Processor state was properly reset" << std::endl;
    }
    
    // Verify instructions were loaded
    bool instructions_loaded = processor.verify_instructions_loaded();
    if (!instructions_loaded) {
        std::cerr << "Failed: Instructions were not loaded" << std::endl;
        return 1;
    } else {
        std::cout << "Passed: Instructions were loaded" << std::endl;
    }
    
    // Verify correct number of instructions
    size_t expected_count = 3; // 3 valid instructions in our test file
    size_t actual_count = processor.get_instruction_count();
    if (actual_count != expected_count) {
        std::cerr << "Failed: Expected " << expected_count << " instructions, got " 
                  << actual_count << std::endl;
        return 1;
    } else {
        std::cout << "Passed: Correct number of instructions loaded (" 
                  << actual_count << ")" << std::endl;
    }
    
    // Verify specific instructions
    uint32_t expected_instr_0 = 0x00500293;
    uint32_t actual_instr_0 = processor.get_instruction(0);
    if (actual_instr_0 != expected_instr_0) {
        std::cerr << "Failed: First instruction mismatch. Expected: 0x" 
                  << std::hex << expected_instr_0 << ", got: 0x" 
                  << actual_instr_0 << std::dec << std::endl;
        return 1;
    } else {
        std::cout << "Passed: First instruction verified (0x" 
                  << std::hex << actual_instr_0 << std::dec << ")" << std::endl;
    }
    
    // Clean up
    std::remove(test_filename.c_str());
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}