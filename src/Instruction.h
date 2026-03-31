#pragma once
#include <string>

enum Opcode {
    OP_NOP,
    OP_ADD,
    OP_ADDI,
    OP_LOAD,
    OP_STORE,
    OP_BNEZ,
    OP_HALT
};

struct Instruction {
    Opcode op = OP_NOP;

    int rd = -1;
    int rs1 = -1;
    int rs2 = -1;
    int imm = 0;

    std::string toString() const;
};