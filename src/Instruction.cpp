#include "Instruction.h"
#include <limits>
#include <sstream>

namespace {
uint32_t maskBits(uint32_t value, int n) {
    return value & ((1u << n) - 1u);
}

int32_t signExtend19(uint32_t x) {
    if (x & (1u << 18)) {
        return static_cast<int32_t>(x | 0xFFF80000u);
    }
    return static_cast<int32_t>(x);
}
}

InstFormat Instruction::format() const {
    switch (op) {
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        case Opcode::DIV:
        case Opcode::MOD:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::NOT:
        case Opcode::SLL:
        case Opcode::SRL:
        case Opcode::SRA:
        case Opcode::SLT:
            return InstFormat::R_TYPE;
        case Opcode::ADDI:
        case Opcode::LW:
        case Opcode::SW:
            return InstFormat::I_TYPE;
        case Opcode::BEQ:
        case Opcode::BNE:
        case Opcode::BLT:
        case Opcode::BGE:
            return InstFormat::BR_TYPE;
        case Opcode::J:
        case Opcode::JAL:
            return InstFormat::J_TYPE;
        case Opcode::NOP:
        case Opcode::HALT:
            return InstFormat::SPECIAL;
    }
    return InstFormat::SPECIAL;
}

bool Instruction::writesRegister() const {
    switch (op) {
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        case Opcode::DIV:
        case Opcode::MOD:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::NOT:
        case Opcode::SLL:
        case Opcode::SRL:
        case Opcode::SRA:
        case Opcode::SLT:
        case Opcode::ADDI:
        case Opcode::LW:
        case Opcode::JAL:
            return true;
        default:
            return false;
    }
}

bool Instruction::readsRs1() const {
    switch (op) {
        case Opcode::NOP:
        case Opcode::HALT:
            return false;
        default:
            return true;
    }
}

bool Instruction::readsRs2() const {
    switch (op) {
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        case Opcode::DIV:
        case Opcode::MOD:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::SLL:
        case Opcode::SRL:
        case Opcode::SRA:
        case Opcode::SLT:
        case Opcode::BEQ:
        case Opcode::BNE:
        case Opcode::BLT:
        case Opcode::BGE:
            return true;
        default:
            return false;
    }
}

bool Instruction::isLoad() const { return op == Opcode::LW; }
bool Instruction::isStore() const { return op == Opcode::SW; }

bool Instruction::isBranch() const {
    return op == Opcode::BEQ || op == Opcode::BNE || op == Opcode::BLT || op == Opcode::BGE;
}

bool Instruction::isJump() const {
    return op == Opcode::J || op == Opcode::JAL;
}

bool Instruction::updatesZeroFlag() const {
    switch (op) {
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        case Opcode::DIV:
        case Opcode::MOD:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::NOT:
        case Opcode::SLL:
        case Opcode::SRL:
        case Opcode::SRA:
        case Opcode::SLT:
        case Opcode::ADDI:
            return true;
        default:
            return false;
    }
}

std::string opcodeToString(Opcode op) {
    switch (op) {
        case Opcode::NOP: return "NOP";
        case Opcode::ADD: return "ADD";
        case Opcode::SUB: return "SUB";
        case Opcode::MUL: return "MUL";
        case Opcode::DIV: return "DIV";
        case Opcode::MOD: return "MOD";
        case Opcode::AND: return "AND";
        case Opcode::OR: return "OR";
        case Opcode::XOR: return "XOR";
        case Opcode::NOT: return "NOT";
        case Opcode::SLL: return "SLL";
        case Opcode::SRL: return "SRL";
        case Opcode::SRA: return "SRA";
        case Opcode::SLT: return "SLT";
        case Opcode::ADDI: return "ADDI";
        case Opcode::LW: return "LW";
        case Opcode::SW: return "SW";
        case Opcode::BEQ: return "BEQ";
        case Opcode::BNE: return "BNE";
        case Opcode::BLT: return "BLT";
        case Opcode::BGE: return "BGE";
        case Opcode::J: return "J";
        case Opcode::JAL: return "JAL";
        case Opcode::HALT: return "HALT";
    }
    return "UNKNOWN";
}

std::string Instruction::toString() const {
    std::ostringstream oss;
    switch (format()) {
        case InstFormat::R_TYPE:
            if (op == Opcode::NOT) {
                oss << opcodeToString(op) << " R" << rd << ", R" << rs1;
            } else {
                oss << opcodeToString(op) << " R" << rd << ", R" << rs1 << ", R" << rs2;
            }
            break;
        case InstFormat::I_TYPE:
            if (op == Opcode::SW) {
                oss << "SW R" << rd << ", R" << rs1 << ", " << imm;
            } else {
                oss << opcodeToString(op) << " R" << rd << ", R" << rs1 << ", " << imm;
            }
            break;
        case InstFormat::BR_TYPE:
            oss << opcodeToString(op) << " R" << rs1 << ", R" << rs2 << ", " << imm;
            break;
        case InstFormat::J_TYPE:
            oss << opcodeToString(op) << " R" << rs1;
            break;
        case InstFormat::SPECIAL:
            oss << opcodeToString(op);
            break;
    }
    return oss.str();
}

uint32_t encodeInstruction(const Instruction& inst) {
    const uint32_t opcode = static_cast<uint32_t>(inst.op);
    uint32_t raw = opcode << 27;
    switch (inst.format()) {
        case InstFormat::R_TYPE:
            raw |= (static_cast<uint32_t>(inst.rd & 0xF) << 23);
            raw |= (static_cast<uint32_t>(inst.rs1 & 0xF) << 19);
            raw |= (static_cast<uint32_t>(inst.rs2 & 0xF) << 15);
            break;
        case InstFormat::I_TYPE:
            raw |= (static_cast<uint32_t>(inst.rd & 0xF) << 23);
            raw |= (static_cast<uint32_t>(inst.rs1 & 0xF) << 19);
            raw |= maskBits(static_cast<uint32_t>(inst.imm), 19);
            break;
        case InstFormat::BR_TYPE:
            raw |= (static_cast<uint32_t>(inst.rs1 & 0xF) << 23);
            raw |= (static_cast<uint32_t>(inst.rs2 & 0xF) << 19);
            raw |= maskBits(static_cast<uint32_t>(inst.imm), 19);
            break;
        case InstFormat::J_TYPE:
            raw |= (static_cast<uint32_t>(inst.rs1 & 0xF) << 23);
            break;
        case InstFormat::SPECIAL:
            break;
    }
    return raw;
}

Instruction decodeInstruction(uint32_t raw) {
    Instruction inst;
    inst.raw = raw;
    inst.op = static_cast<Opcode>((raw >> 27) & 0x1F);

    switch (inst.format()) {
        case InstFormat::R_TYPE:
            inst.rd = static_cast<int>((raw >> 23) & 0xF);
            inst.rs1 = static_cast<int>((raw >> 19) & 0xF);
            inst.rs2 = static_cast<int>((raw >> 15) & 0xF);
            break;
        case InstFormat::I_TYPE:
            inst.rd = static_cast<int>((raw >> 23) & 0xF);
            inst.rs1 = static_cast<int>((raw >> 19) & 0xF);
            inst.imm = signExtend19(raw & 0x7FFFF);
            break;
        case InstFormat::BR_TYPE:
            inst.rs1 = static_cast<int>((raw >> 23) & 0xF);
            inst.rs2 = static_cast<int>((raw >> 19) & 0xF);
            inst.imm = signExtend19(raw & 0x7FFFF);
            break;
        case InstFormat::J_TYPE:
            inst.rs1 = static_cast<int>((raw >> 23) & 0xF);
            break;
        case InstFormat::SPECIAL:
            break;
    }
    return inst;
}
