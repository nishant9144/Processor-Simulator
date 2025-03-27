#include "no_forward_processor.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        // Create processor instance
        NoForwardingProcessor* processor = new NoForwardingProcessor();

        processor->load_program(argv[1]);

        processor->run_simulation(20);

        processor->print_pipeline_diagram();

        delete processor;

    } catch (const std::exception& e) {
        std::cerr << "Error during simulation: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
