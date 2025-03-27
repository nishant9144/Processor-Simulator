#include "no_forward_processor.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    try {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " <program_file> <num_cycles>" << std::endl;
            return 1;
        }

        std::filesystem::path inputPath(argv[1]);
        std::string baseFilename = inputPath.stem().string();
        mkdir("../outputfiles", 0777);
        std::string outputFilename = "../outputfiles/" + baseFilename + "_noforward_out.txt";
        FILE* outputFile = freopen(outputFilename.c_str(), "w", stdout);
        if (!outputFile) {
            std::cerr << "Error: Could not open output file " << outputFilename << std::endl;
            return 1;
        }


        NoForwardingProcessor* processor = new NoForwardingProcessor();
        int num_cycles = atoi(argv[2]);
        
        processor->load_program(argv[1]);

        processor->run_simulation(num_cycles);        
        
        processor->print_pipeline_diagram();

        // Close the output file
        fclose(outputFile);
        
        delete processor;

    } catch (const std::exception& e) {
        std::cerr << "Error during simulation: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}