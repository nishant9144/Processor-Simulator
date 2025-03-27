#include "forward_processor.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        // Create processor instance
        ForwardingProcessor* processor = new ForwardingProcessor();

        processor->load_program(argv[1]);

        int num_cycles = atoi(argv[2]);

        processor->run_simulation(num_cycles);

        processor->print_pipeline_diagram();

        delete processor;

    } catch (const std::exception& e) {
        std::cerr << "Error during simulation: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
