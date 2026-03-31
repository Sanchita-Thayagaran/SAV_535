#include "Instruction.h"
#include <sstream>

std::string Instruction::toString() const {
    std::ostringstream oss;

    switch (op) {
        case OP_ADD:
            oss << "ADD R" << rd << " R" << rs1 << " R" << rs2;
            break;

        case OP_ADDI:
            oss << "ADDI R" << rd << " R" << rs1 << " " << imm;
            break;

        case OP_LOAD:
            oss << "LOAD R" << rd << " R" << rs1 << " " << imm;
            break;

        case OP_STORE:
            oss << "STORE R" << rs2 << " R" << rs1 << " " << imm;
            break;

        case OP_BNEZ:
            oss << "BNEZ R" << rs1 << " " << imm;
            break;

        case OP_HALT:
            oss << "HALT";
            break;

        default:
            oss << "NOP";
    }

    return oss.str();
}