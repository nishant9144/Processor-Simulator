#ifndef DS_HPP
#define DS_HPP

#include <vector>
#include <map>
#include <cstdint>

typedef long long ll;
using namespace std;

struct instruction_memory
{
    vector<uint32_t> instructions;
    // map<Address, std::string> instruction_strings;  // For displaying mnemonics
};

struct register_memory
{
    uint64_t registers[32] = {0};

    uint64_t read(uint8_t reg)
    {
        return (reg == 0) ? 0 : registers[reg];
    }

    void write(uint8_t reg, uint64_t value)
    {
        if (reg != 0)
            registers[reg] = value;
    }
};
struct data_memory
{
    map<uint64_t, uint64_t> data_memory;

    uint64_t read(uint64_t addr)
    {
        return data_memory[addr];
    }
    void write(uint64_t addr, uint64_t value)
    {
        data_memory[addr] = value;
    }
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
    // bool jump = false;        // For j/jal instructions
    // bool jumpReg = false;     // For jalr instruction
};

struct IF_ID_register_file
{
    uint32_t instruction;
    uint64_t program_counter;

    bool stall = false;
    bool flush = false;
};

struct ID_EX_register_file
{
    int WB[2] = {0};
    int M[3] = {0};
    int EX[2] = {0};

    int64_t readData1 = 0;
    int64_t readData2 = 0;

    int64_t immediate = 0;

    uint8_t IF_ID_Register_RS1; // This all will be a 5-bit number for the registers.
    uint8_t IF_ID_Register_RS2;
    uint8_t IF_ID_Register_RD;
};

struct EX_MEM_register_file
{
    int WB[2] = {0};
    int M[3] = {0};

    int64_t alu_result;
    int64_t write_data;

    // bool zero = false;

    uint8_t ID_EX_RegisterRD; // This will be a 5-bit number
};

struct MEM_WB_register_file
{
    int WB[2] = {0};

    int64_t alu_result = 0;
    int64_t read_data = 0;

    uint64_t EX_MEM_RegisterRD; // This will be a 5-bit number
};

struct program_counter
{
    uint32_t instruction_address = 0;
};

// Forwarding unit signals (for the forwarding processor)
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

// Hazard detection unit
struct HazardDetectionUnit
{
    bool stall = false;

    void detect(bool id_ex_memRead, uint32_t id_ex_rd, uint32_t if_id_rs1, uint32_t if_id_rs2)
    {
        // Load-use hazard
        stall = id_ex_memRead && (id_ex_rd == if_id_rs1 || id_ex_rd == if_id_rs2);
    }
};

// ALU class for the execute stage
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

    static uint64_t compute(uint64_t a, uint64_t b, Operation op)
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
            return a >> (b & 0x3F);
        case Operation::SRA:
            return static_cast<int64_t>(a) >> (b & 0x3F);
        case Operation::SLT:
            return (static_cast<int64_t>(a) < static_cast<int64_t>(b)) ? 1 : 0;
        case Operation::SLTU:
            return (a < b) ? 1 : 0;
        default:
            return 0;
        }
    }

    static bool isZero(uint64_t result)
    {
        return result == 0;
    }
};

#endif // DS_HPP