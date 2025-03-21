#include "no_forward_processor.hpp"
#include "forward_processor.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <instruction_file> <processor_type>\n";
        std::cerr << "  processor_type: 0 for no forwarding, 1 for forwarding\n";
        return 1;
    }
    
    std::string filename = argv[1];
    int processor_type = std::stoi(argv[2]);
    
    Processor* processor = nullptr;
    
    if (processor_type == 0) {
        processor = new NoForwardingProcessor();
        std::cout << "Running simulation with no forwarding processor\n";
    } else {
        processor = new ForwardingProcessor();
        std::cout << "Running simulation with forwarding processor\n";
    }
    
    processor->load_program(filename);
    processor->run_simulation(100); 
    processor->print_pipeline_diagram();
    
    delete processor;
    return 0;
}