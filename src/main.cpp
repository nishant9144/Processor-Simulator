#include "no_forward_processor.hpp"
#include "forward_processor.hpp"
#include <iostream>
#include <string>

int main() {
    try {
        // Create processor instance
        NoForwardingProcessor* processor = new NoForwardingProcessor();

        // Load test program
        processor->load_program("input.txt");

        // Run simulation with a reasonable max cycle limit
        // The number of cycles should be at least the number of instructions + some extra for pipeline stages
        processor->run_simulation(100);

        // Print pipeline diagram and final state
        processor->print_pipeline_diagram();

        delete processor;

    } catch (const std::exception& e) {
        std::cerr << "Error during simulation: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

/*
new

00a282b3 add x5 x5 x10;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a28133 add x2 x5 x10;   ;IF ;IF ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a282b3 add x5 x5 x10;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a281b3 add x3 x5 x10;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - 
00a28233 add x4 x5 x10;   ;   ;   ;   ;   ;   ;IF ;IF ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - 
00a284b3 add x9 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - 
00a28333 add x6 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - 
00a283b3 add x7 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - 
00a28433 add x8 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - 

00a282b3 add x5 x5 x10;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a28133 add x2 x5 x10;   ;IF ;IF ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a282b3 add x5 x5 x10;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a281b3 add x3 x5 x10;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - 
00a28233 add x4 x5 x10;   ;   ;   ;   ;   ;   ;IF ;IF ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - 
00a284b3 add x9 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - 
00a28333 add x6 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - 
00a283b3 add x7 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - 
00a28433 add x8 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - 

00a282b3 add x5 x5 x10;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a28133 add x2 x5 x10;   ;IF ;IF ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a282b3 add x5 x5 x10;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a281b3 add x3 x5 x10;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - 
00a28233 add x4 x5 x10;   ;   ;   ;   ;   ;   ;IF ;IF ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - 
00a284b3 add x9 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - 
00a28333 add x6 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - 
00a283b3 add x7 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - 
00a28433 add x8 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - 

old
00a282b3 add x5 x5 x10;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a28133 add x2 x5 x10;   ;IF ;IF ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a282b3 add x5 x5 x10;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - ; - 
00a281b3 add x3 x5 x10;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - ; - ; - ; - 
00a28233 add x4 x5 x10;   ;   ;   ;   ;   ;   ;IF ;IF ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - ; - 
00a284b3 add x9 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - ; - 
00a28333 add x6 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - ; - 
00a283b3 add x7 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - ; - 
00a28433 add x8 x5 x10;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;   ;IF ;ID ;EX ;MEM;WB ; - 


*/