# **Processor Simulator**

This project implements a 5-stage pipelined RISC-V processor simulator with support for both forwarding and non-forwarding execution modes.

## **Overview**

The processor simulator is designed to demonstrate the principles of pipelined CPU execution with a focus on data hazard handling. It includes:

* A 5-stage pipeline (Fetch, Decode, Execute, Memory, Write-back)
* Two execution modes:
  * Forwarding: Resolves data hazards through register value forwarding
  * No-forwarding: Resolves data hazards through pipeline stalls
* Branch handling in the Decode stage for better performance
* Pipeline visualization for analysis
* Configurable simulation parameters

## **Architecture**

The simulator implements the following core RISC-V instruction types:

* R-type: ADD, SUB, AND, OR, XOR, SLL, SRL, SRA, SLT, SLTU
* I-type: ADDI, ANDI, ORI, XORI, SLLI, SRLI, SRAI, SLTI, SLTIU, LW, LB, LH
* S-type: SW, SB, SH
* B-type: BEQ, BNE
* J-type: JAL
* Special: JALR

## **Data Structures**

```
🔹 instruction_memory : Stores the program instructions
🔹 register_memory    : 32 general-purpose registers
🔹 data_memory        : Memory for load/store operations

Pipeline registers:
🔹 IF_ID_register_file : Between Fetch and Decode stages
🔹 ID_EX_register_file : Between Decode and Execute stages
🔹 EX_MEM_register_file: Between Execute and Memory stages
🔹 MEM_WB_register_file: Between Memory and Write-back stages
```

## **Hazard Handling**

### **Forwarding Processor**

* Uses a ForwardingUnit to detect and resolve data dependencies
* Only stalls for load-use hazards (when an instruction depends on the result of a load)
* Implements bypass paths from:
  * EX/MEM stage (results from previous instruction)
  * MEM/WB stage (results from two instructions prior)

### **Non-Forwarding Processor**

* Detects all RAW (Read-After-Write) hazards
* Inserts stalls whenever a hazard is detected
* Introduces more pipeline bubbles, but has simpler hardware requirements

## **Branch Handling**

* Branch resolution occurs in the Decode stage
* The processor detects flush conditions for incorrect branch predictions
* Branches are evaluated early to minimize branch penalties

## **Usage**

### **Building the Simulator**

```
cd src
make
```

### **Running a Simulation**

```
./a.out
```

The simulator will load instructions from input.txt by default and run the simulation.

### **Using Different Test Files**

Edit main.cpp to change the input file path, or provide it as a command-line argument:

```cpp
processor->load_program("test_add.txt");  // For testing addition
processor->load_program("test_load.txt"); // For testing load operations
processor->load_program("test_all.txt");  // For testing all instructions
```

## **Testing**

The project includes comprehensive unit tests for different components:

* Control signal generation
* ALU operation selection
* Instruction loading
* Program loading

Run tests individually:

```
g++ -std=c++17 tests/test_generate_control_signals.cpp src/processor.cpp -o control_test
./control_test
```

## **Pipeline Visualization**

The simulator generates a pipeline execution diagram showing the progress of each instruction through the pipeline stages:

```
00a282b3 add x5 x5 x10;IF;ID;EX;MEM;WB;-;-;-;-;-;-;-;-;-;-
00a28133 add x2 x5 x10; ;IF;IF;IF;ID;EX;MEM;WB;-;-;-;-;-;-;-
00a281b3 add x3 x5 x10; ; ; ; ;IF;ID;EX;MEM;WB;-;-;-;-;-;-
```

Each row represents an instruction, and each column shows which pipeline stage the instruction was in during each clock cycle. Stalls and flushes are also represented.

## **Pipeline Architecture Diagram**

![RISC-V 5-Stage Pipeline Processor Diagram](images/pipeline-diagram.png)

The comprehensive pipeline architecture diagram illustrates the intricate connections and data flow between different stages of the RISC-V processor. Key components and their interactions are visualized:

* **Instruction Fetch (IF) Stage**: 
  - Retrieves instructions from instruction memory
  - Manages program counter and instruction address generation
  - Connects to IF/ID pipeline register

* **Instruction Decode (ID) Stage**:
  - Decodes incoming instructions
  - Reads register values
  - Contains hazard detection unit
  - Manages branch prediction and resolution
  - Connects to ID/EX pipeline register

* **Execute (EX) Stage**:
  - Performs arithmetic and logical operations
  - Contains ALU (Arithmetic Logic Unit)
  - Implements forwarding unit for data hazard resolution
  - Handles immediate value processing
  - Connects to EX/MEM pipeline register

* **Memory (MEM) Stage**:
  - Manages memory read and write operations
  - Handles data memory access
  - Supports different memory access types (load/store)
  - Connects to MEM/WB pipeline register

* **Write Back (WB) Stage**:
  - Writes back results to register file
  - Completes instruction execution cycle

**Key Connections**:
- Dotted orange lines represent control signals
- Thick black lines represent data paths
- Pipeline registers (IF/ID, ID/EX, EX/MEM, MEM/WB) manage state between stages
- Forwarding unit and hazard detection unit ensure efficient instruction processing

The diagram provides a comprehensive view of how instructions flow through the processor, highlighting the complex interactions between different components and stages of the pipeline.