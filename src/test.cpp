// below processor.cpp

int main() {
    try {
        // Create processor instance
        Processor processor;

        // Load test program
        processor.load_program("input.txt");

        // Run simulation with a reasonable max cycle limit
        // The number of cycles should be at least the number of instructions + some extra for pipeline stages
        processor.run_simulation(100);

        // Print pipeline diagram and final state
        processor.print_pipeline_diagram();

    } catch (const std::exception& e) {
        std::cerr << "Error during simulation: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}