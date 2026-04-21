#include "Assembler.h"
#include "Parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<std::string> Assembler::readLines(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        throw std::runtime_error("Could not open assembly file: " + filename);
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) lines.push_back(line);
    return lines;
}

std::vector<std::string> Assembler::preprocess(const std::vector<std::string>& lines) {
    std::vector<std::string> out;
    out.reserve(lines.size());
    for (const auto& raw : lines) {
        std::string line = Parser::stripComment(raw);
        if (!line.empty()) out.push_back(line);
    }
    return out;
}

std::unordered_map<std::string, uint32_t> Assembler::collectLabels(const std::vector<std::string>& lines) {
    std::unordered_map<std::string, uint32_t> labels;
    uint32_t pc = 0;
    for (const auto& raw : lines) {
        std::string line = raw;
        while (true) {
            auto colon = line.find(':');
            if (colon == std::string::npos) break;
            std::string label = Parser::trim(line.substr(0, colon));
            if (label.empty()) throw std::runtime_error("Empty label");
            labels[Parser::upper(label)] = pc;
            line = Parser::trim(line.substr(colon + 1));
            if (line.empty()) break;
        }
        if (!line.empty()) ++pc;
    }
    return labels;
}

int Assembler::parseRegister(const std::string& token) {
    std::string t = Parser::upper(token);
    if (t.size() < 2 || t[0] != 'R') throw std::runtime_error("Expected register, got: " + token);
    int reg = std::stoi(t.substr(1));
    if (reg < 0 || reg > 15) throw std::runtime_error("Register out of range: " + token);
    return reg;
}

int Assembler::parseImm(const std::string& token) {
    int base = 10;
    if (token.size() > 2 && token[0] == '0' && (token[1] == 'X' || token[1] == 'x')) base = 16;
    return std::stoi(token, nullptr, base);
}

int Assembler::parseAbsoluteOrNumber(const std::string& token,
                                     const std::unordered_map<std::string, uint32_t>& labels) {
    const std::string u = Parser::upper(token);
    if (labels.count(u)) return static_cast<int>(labels.at(u));
    return parseImm(token);
}

int Assembler::parseBranchOffset(const std::string& token,
                                 uint32_t pc,
                                 const std::unordered_map<std::string, uint32_t>& labels) {
    const std::string u = Parser::upper(token);
    if (labels.count(u)) {
        return static_cast<int>(labels.at(u)) - static_cast<int>(pc + 1);
    }
    return parseImm(token);
}

Instruction Assembler::parseInstruction(
    const std::string& input,
    uint32_t pc,
    const std::unordered_map<std::string, uint32_t>& labels
) {
    std::string line = input;
    while (true) {
        auto colon = line.find(':');
        if (colon == std::string::npos) break;
        line = Parser::trim(line.substr(colon + 1));
    }
    const auto toks = Parser::tokenize(line);
    if (toks.empty()) return {};

    const std::string op = Parser::upper(toks[0]);
    Instruction inst;

    auto require = [&](size_t n) {
        if (toks.size() != n) {
            throw std::runtime_error("Wrong operand count for line: " + line);
        }
    };

    if (op == "RET") {
        inst.op = Opcode::J;
        inst.rs1 = 15;
    } else if (op == "LA") {
        require(3);
        inst.op = Opcode::ADDI;
        inst.rd = parseRegister(toks[1]);
        inst.rs1 = 0;
        inst.imm = parseAbsoluteOrNumber(toks[2], labels);
    } else if (op == "BNEZ") {
        require(3);
        inst.op = Opcode::BNE;
        inst.rs1 = parseRegister(toks[1]);
        inst.rs2 = 0;
        inst.imm = parseBranchOffset(toks[2], pc, labels);
    } else if (op == "NOP") {
        inst.op = Opcode::NOP;
    } else if (op == "HALT") {
        inst.op = Opcode::HALT;
    } else if (op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV" ||
               op == "MOD" || op == "AND" || op == "OR"  || op == "XOR" ||
               op == "SLL" || op == "SRL" || op == "SRA" || op == "SLT") {
        require(4);
        inst.op = static_cast<Opcode>(
            op == "ADD" ? Opcode::ADD :
            op == "SUB" ? Opcode::SUB :
            op == "MUL" ? Opcode::MUL :
            op == "DIV" ? Opcode::DIV :
            op == "MOD" ? Opcode::MOD :
            op == "AND" ? Opcode::AND :
            op == "OR"  ? Opcode::OR  :
            op == "XOR" ? Opcode::XOR :
            op == "SLL" ? Opcode::SLL :
            op == "SRL" ? Opcode::SRL :
            op == "SRA" ? Opcode::SRA : Opcode::SLT
        );
        inst.rd  = parseRegister(toks[1]);
        inst.rs1 = parseRegister(toks[2]);
        inst.rs2 = parseRegister(toks[3]);
    } else if (op == "NOT") {
        require(3);
        inst.op = Opcode::NOT;
        inst.rd  = parseRegister(toks[1]);
        inst.rs1 = parseRegister(toks[2]);
    } else if (op == "ADDI" || op == "LW" || op == "SW") {
        require(4);
        inst.op = op == "ADDI" ? Opcode::ADDI : (op == "LW" ? Opcode::LW : Opcode::SW);
        inst.rd  = parseRegister(toks[1]);   // for SW: value register
        inst.rs1 = parseRegister(toks[2]);   // base register
        inst.imm = op == "ADDI"
            ? parseAbsoluteOrNumber(toks[3], labels)
            : parseImm(toks[3]);
    } else if (op == "BEQ" || op == "BNE" || op == "BLT" || op == "BGE") {
        require(4);
        inst.op = op == "BEQ" ? Opcode::BEQ :
                  op == "BNE" ? Opcode::BNE :
                  op == "BLT" ? Opcode::BLT : Opcode::BGE;
        inst.rs1 = parseRegister(toks[1]);
        inst.rs2 = parseRegister(toks[2]);
        inst.imm = parseBranchOffset(toks[3], pc, labels);
    } else if (op == "J" || op == "JAL") {
        require(2);
        inst.op = op == "J" ? Opcode::J : Opcode::JAL;
        inst.rs1 = parseRegister(toks[1]);
    } else {
        throw std::runtime_error("Unknown opcode: " + op);
    }

    inst.raw = encodeInstruction(inst);
    return inst;
}

AssemblyResult Assembler::assembleLines(const std::vector<std::string>& rawLines) {
    const auto lines = preprocess(rawLines);
    const auto labels = collectLabels(lines);

    AssemblyResult out;
    uint32_t pc = 0;
    for (const auto& line : lines) {
        std::string stripped = line;
        while (true) {
            auto colon = stripped.find(':');
            if (colon == std::string::npos) break;
            stripped = Parser::trim(stripped.substr(colon + 1));
        }
        if (stripped.empty()) continue;

        Instruction inst = parseInstruction(line, pc, labels);
        uint32_t word = encodeInstruction(inst);
        out.words.push_back(word);
        out.listing.push_back({pc, word, stripped});
        ++pc;
    }
    return out;
}

AssemblyResult Assembler::assembleFile(const std::string& filename) {
    return assembleLines(readLines(filename));
}
