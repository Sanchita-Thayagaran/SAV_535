#pragma once
#include <cstdint>
#include <string>

enum class Opcode : uint8_t {
    NOP  = 0b00000,
    ADD  = 0b00001,
    SUB  = 0b00010,
    MUL  = 0b00011,
    DIV  = 0b00100,
    MOD  = 0b00101,
    AND  = 0b00110,
    OR   = 0b00111,
    XOR  = 0b01000,
    NOT  = 0b01001,
    SLL  = 0b01010,
    SRL  = 0b01011,
    SRA  = 0b01100,
    SLT  = 0b01101,
    ADDI = 0b01110,
    LW   = 0b01111,
    SW   = 0b10000,
    BEQ  = 0b10001,
    BNE  = 0b10010,
    BLT  = 0b10011,
    BGE  = 0b10100,
    J    = 0b10101,
    JAL  = 0b10110,
    HALT = 0b11111
};

enum class InstFormat {
    R_TYPE,
    I_TYPE,
    BR_TYPE,
    J_TYPE,
    SPECIAL
};

struct Instruction {
    Opcode op = Opcode::NOP;
    int rd = 0;
    int rs1 = 0;
    int rs2 = 0;
    int imm = 0;
    uint32_t raw = 0;

    InstFormat format() const;
    bool writesRegister() const;
    bool readsRs1() const;
    bool readsRs2() const;
    bool isLoad() const;
    bool isStore() const;
    bool isBranch() const;
    bool isJump() const;
    bool updatesZeroFlag() const;
    std::string toString() const;
};

uint32_t encodeInstruction(const Instruction& inst);
Instruction decodeInstruction(uint32_t raw);
std::string opcodeToString(Opcode op);
