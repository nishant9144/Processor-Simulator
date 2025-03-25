#ifndef DS_HPP
#define DS_HPP

#include <iostream>
#include <vector>
#include <map>
#include <cstdint>

typedef long long ll;
using namespace std;

struct program_counter
{
    uint64_t instruction_address = 0;
    bool stall = false;

    void update(uint64_t nextPC){   // this function gets nextPC from PC_handler and then updates the PC based on the stall<bool>.
        if(!stall){
            instruction_address = nextPC;
        }
    }
};

struct instruction_memory
{
    uint64_t address = 0;
    vector<uint32_t> instructions;
    uint32_t instruction = 0;
    void fetch()
    {
        if (address / 4 >= instructions.size())
        {
            cerr << "Invalid instruction address!\n";
            exit(1);
        }
        else
        {
            instruction = instructions[address / 4];
        }
    }

    // map<Address, std::string> instruction_strings;  // For displaying mnemonics
};

struct PC_handler
{
    uint64_t currPC = -4;
    uint64_t branch_jump_PC = 0;
    bool is_branch_jump = 0;
    bool branch_taken = 0;

    uint64_t handle(){
        if(is_branch_jump && branch_taken){
            return (branch_jump_PC + currPC);
        }else{
            return (currPC + 4);
        }
    }

    PC_handler(){}
};


struct IF_ID_register_file
{
    uint32_t instruction = 0;
    uint64_t program_counter = 0;

    bool flush = false;

    // Constructor
    IF_ID_register_file() {}
};

struct ControlSignals
{
    // WB control signals
    bool regWrite = false;
    bool memToReg = false;

    // MEM control signals
    bool memRead = false;
    bool memWrite = false;
    bool branch = false;

    // EX control signals
    bool aluSrc = false;
    uint8_t aluOp = 0;

    // bool jump    = false;        // For j/jal instructions
    // bool jumpReg = false;        // For jalr instruction

    // Constructor
    ControlSignals() {}
};

// struct HazardDetectionUnit
// {
//     uint64_t instruction;
//     bool stall = false;
//     bool flush = false;

//     // bool beq = false; Or maybe just extract from the instruction

//     // For stalling
//     void detect(bool id_ex_memRead, uint32_t id_ex_rd, uint32_t if_id_rs1, uint32_t if_id_rs2)
//     {
//         // Load-use hazard
//         stall = id_ex_memRead && (id_ex_rd == if_id_rs1 || id_ex_rd == if_id_rs2);
//     }
// };

struct HazardDetectionUnit
{
    bool stall = false;
    bool flush = false;
    uint32_t instruction = 0;
    bool is_equal = false;

    // For no-forwarding processor - detect all RAW hazards
    void detect(uint8_t id_ex_rd, uint8_t ex_mem_rd, uint8_t mem_wb_rd,
                uint8_t if_id_rs1, uint8_t if_id_rs2, bool id_ex_memRead)
    {
        // Any instruction in ID stage depends on result from previous instructions
        bool hazard_id_ex = (id_ex_rd != 0) &&
                            (id_ex_rd == if_id_rs1 || id_ex_rd == if_id_rs2);

        bool hazard_ex_mem = (ex_mem_rd != 0) &&
                             (ex_mem_rd == if_id_rs1 || ex_mem_rd == if_id_rs2);

        // Stall if any hazard is detected
        stall = hazard_id_ex || hazard_ex_mem;

        uint32_t opcode = instruction & 0x7F;

        if (opcode == 0x67 and is_equal)
        {
            flush = true;
            stall = false;
        }
    }
};

struct register_memory
{
    // Inputs
    uint8_t r1 = 0;
    uint8_t r2 = 0;
    uint8_t rd = 0;
    int64_t w_data = 0;

    // Control signals
    bool regWrite = false;
    bool branch_eq = false;

    // Register memory
    int64_t registers[32] = {0};

    // Outputs
    int64_t r_data1 = 0;
    int64_t r_data2 = 0;

    void write()
    {
        if (regWrite)
        {
            rd = (rd & 1) + (rd & 2) * 2 + (rd & 4) * 4 + (rd & 8) * 8 + (rd & 16) * 16; // converted to a valid index
            if (rd != 0)
            { // x0 shouldn't be changed
                registers[rd] = w_data;
            }
        }
    }
    void produce_read()
    {
        r1 = (r1 & 1) + (r1 & 2) * 2 + (r1 & 4) * 4 + (r1 & 8) * 8 + (r1 & 16) * 16;
        r2 = (r2 & 1) + (r2 & 2) * 2 + (r2 & 4) * 4 + (r2 & 8) * 8 + (r2 & 16) * 16;
        r_data1 = registers[r1];
        r_data2 = registers[r2];
        branch_eq = (r_data1 == r_data2);
    }
};

struct imm_gen
{
    uint32_t instruction = 0;
    int64_t extended = 0;

    void generate()
    {
        // Extract opcode
        uint32_t opcode = instruction & 0x7F;

        // I-type: Load, ALU immediate, JALR
        if ((opcode == 0x03) || (opcode == 0x13) || (opcode == 0x67))
        {
            // imm[11:0] = inst[31:20]
            int64_t imm = ((int32_t)(instruction & 0xFFF00000)) >> 20;
            extended = imm;
        }

        // S-type: Store instructions
        else if (opcode == 0x23)
        {
            // imm[11:5] = inst[31:25], imm[4:0] = inst[11:7]
            int64_t imm = ((int32_t)(instruction & 0xFE000000)) >> 20;
            imm |= ((instruction >> 7) & 0x1F);
            extended = imm;
        }

        // B-type: Branch instructions
        else if (opcode == 0x63)
        {
            // imm[12|10:5|4:1|11] = inst[31|30:25|11:8|7]
            int64_t imm = ((int32_t)(instruction & 0x80000000)) >> 19;
            imm |= ((instruction & 0x7E000000) >> 20);
            imm |= ((instruction & 0x00000F00) >> 7);
            imm |= ((instruction & 0x00000080) << 4);
            extended = imm;
        }

        // U-type: LUI, AUIPC
        else if ((opcode == 0x17) || (opcode == 0x37))
        {
            // imm[31:12] = inst[31:12]
            int64_t imm = (int32_t)(instruction & 0xFFFFF000);
            extended = imm;
        }

        // J-type: JAL
        else if (opcode == 0x6F)
        {
            // imm[20|10:1|11|19:12] = inst[31|30:21|20|19:12]
            int64_t imm = ((int32_t)(instruction & 0x80000000)) >> 11;
            imm |= (instruction & 0x000FF000);
            imm |= ((instruction & 0x00100000) >> 9);
            imm |= ((instruction & 0x7FE00000) >> 20);
            extended = imm;
        }

        // Default case: return 0
        extended = (int64_t)instruction;
    }
};

struct ID_EX_register_file
{
    // WB control signals
    bool regWrite = false;
    bool memToReg = false;

    // MEM control signals
    bool memRead = false;
    bool memWrite = false;

    // EX control signals
    bool aluSrc = false;
    uint8_t aluOp = 0;

    int64_t readData1 = 0;
    int64_t readData2 = 0;

    int64_t immediate = 0;

    uint8_t IF_ID_Register_RS1 = 0;
    uint8_t IF_ID_Register_RS2 = 0;
    uint8_t IF_ID_Register_RD = 0;

    uint32_t instruction = 0;

    // Constructor - all members already have initializers
    ID_EX_register_file() {}
};

class ALU
{
public:
    enum class Operation
    {
        ADD,
        SUB,
        AND,
        OR,
        XOR,
        SLL,
        SRL,
        SRA,
        SLT,
        SLTU
    };

    static int64_t compute(int64_t a, int64_t b, Operation op)
    {
        switch (op)
        {
        case Operation::ADD:
            return a + b;
        case Operation::SUB:
            return a - b;
        case Operation::AND:
            return a & b;
        case Operation::OR:
            return a | b;
        case Operation::XOR:
            return a ^ b;
        case Operation::SLL:
            return a << (b & 0x3F); // Shift amount is 6 bits
        case Operation::SRL:
            return static_cast<uint64_t>(a) >> (b & 0x3F);
        case Operation::SRA:
            return a >> (b & 0x3F);
        case Operation::SLT:
            return (a < b) ? 1 : 0;
        case Operation::SLTU:
            return (static_cast<uint64_t>(a) < static_cast<uint64_t>(b)) ? 1 : 0;
        default:
            return 0;
        }
    }
};

struct ForwardingUnit
{
    enum class ForwardSource
    {
        NONE,
        EX_MEM,
        MEM_WB
    };

    ForwardSource forwardA = ForwardSource::NONE;
    ForwardSource forwardB = ForwardSource::NONE;

    // Detect forwarding conditions
    void detect(
        bool ex_mem_regWrite, uint32_t ex_mem_rd,
        bool mem_wb_regWrite, uint32_t mem_wb_rd,
        uint32_t id_ex_rs1, uint32_t id_ex_rs2)
    {
        // Forward from EX/MEM
        if (ex_mem_regWrite && ex_mem_rd != 0)
        {
            if (ex_mem_rd == id_ex_rs1)
                forwardA = ForwardSource::EX_MEM;
            if (ex_mem_rd == id_ex_rs2)
                forwardB = ForwardSource::EX_MEM;
        }

        // Forward from MEM/WB
        if (mem_wb_regWrite && mem_wb_rd != 0)
        {
            if (mem_wb_rd == id_ex_rs1 && forwardA != ForwardSource::EX_MEM)
                forwardA = ForwardSource::MEM_WB;
            if (mem_wb_rd == id_ex_rs2 && forwardB != ForwardSource::EX_MEM)
                forwardB = ForwardSource::MEM_WB;
        }
    }
};

struct EX_MEM_register_file
{
    // WB control signals
    bool regWrite = false;
    bool memToReg = false;

    // MEM control signals
    bool memRead = false;
    bool memWrite = false;
    // bool branch   = false;

    int64_t alu_result = 0;
    int64_t write_data = 0;
    uint8_t ID_EX_RegisterRD = 0;

    // bool zero = false;
    EX_MEM_register_file() {}
};

struct data_memory
{
    // Inputs
    uint64_t addr = 0;
    int64_t w_data = 0;

    // Control Signals
    bool memWrite = false;
    bool memRead = false;

    // Memory
    map<uint64_t, int64_t> data_memory;

    // Output
    int64_t r_data = 0;

    void read()
    {
        if (memRead)
        {
            if (data_memory.find(addr) == data_memory.end())
            { // This will be a 5-bit number
                cerr << "Invalid Memory Access in Data Memory.";
                exit(1);
            }
            else
            {
                r_data = data_memory[addr];
            }
        }
    }
    void write()
    {
        if (memWrite)
        {
            data_memory[addr] = w_data;
        }
    }
};

struct MEM_WB_register_file
{
    // WB control signals
    bool regWrite = false;
    bool memToReg = false;

    int64_t alu_result = 0;
    int64_t read_data = 0;
    uint8_t EX_MEM_RegisterRD = 0;

    // Constructor
    MEM_WB_register_file() {}
};

#endif // DS_HPP