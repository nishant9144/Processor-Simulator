#include "../src/processor.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>

// Test class to expose protected methods
class TestProcessor : public Processor {
public:
    void test_load_instructions(const string &filename) {
        load_instructions(filename);
    }
    
    // Accessors to check internal state
    const vector<uint64_t>& get_instructions() const {
        return instr_mem.instructions;
    }
    
    const vector<string>& get_instruction_strings() const {
        return instruction_strings;
    }
};

// Create a test file with sample instructions
void create_test_file(const string &filename) {
    std::ofstream file(filename);
    file << "# This is a comment and should be skipped\n";
    file << "\n";  // Empty line
    file << "00500293  # addi x5, x0, 5\n";
    file << "00a08433  # add x8, x1, x10\n";
    file << "// Another comment style\n";
    file << "00044503  # lbu x10, 0(x8)\n";
    file << "00050a63  # beq x10, x0, 20\n";
    file.close();
}

int main() {
    // Create the test file
    string test_filename = "test_instructions.txt";
    create_test_file(test_filename);
    
    // Create the test processor
    TestProcessor processor;
    
    // Test loading instructions
    processor.test_load_instructions(test_filename);
    
    // Verify the instructions were loaded correctly
    const auto& instructions = processor.get_instructions();
    const auto& instruction_strings = processor.get_instruction_strings();
    
    // Check number of instructions loaded (should be 4, skipping comments and empty lines)
    assert(instructions.size() == 4);
    assert(instruction_strings.size() == 4);
    
    // Check actual instruction values
    assert(instructions[0] == 0x00500293);
    assert(instructions[1] == 0x00a08433);
    assert(instructions[2] == 0x00044503);
    assert(instructions[3] == 0x00050a63);
    
    // Check instruction strings were preserved
    assert(instruction_strings[0].find("00500293") != string::npos);
    assert(instruction_strings[1].find("00a08433") != string::npos);
    assert(instruction_strings[2].find("00044503") != string::npos);
    assert(instruction_strings[3].find("00050a63") != string::npos);
    
    // Clean up
    std::remove(test_filename.c_str());
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}