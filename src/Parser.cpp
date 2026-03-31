#include "Parser.h"
#include <fstream>
#include <sstream>
#include <iostream>

// helper: convert "R4" → 4
int parseReg(const std::string& r) {
    if (r[0] == 'R' || r[0] == 'r')
        return std::stoi(r.substr(1));
    return std::stoi(r);
}

Instruction parseLine(const std::string& line) {
    std::istringstream iss(line);
    std::string op;
    iss >> op;

    Instruction inst;

    if (op == "ADD") {
        inst.op = OP_ADD;
        std::string rd, rs1, rs2;
        iss >> rd >> rs1 >> rs2;
        inst.rd = parseReg(rd);
        inst.rs1 = parseReg(rs1);
        inst.rs2 = parseReg(rs2);
    }
    else if (op == "ADDI") {
        inst.op = OP_ADDI;
        std::string rd, rs1;
        iss >> rd >> rs1 >> inst.imm;
        inst.rd = parseReg(rd);
        inst.rs1 = parseReg(rs1);
    }
    else if (op == "LOAD") {
        inst.op = OP_LOAD;
        std::string rd, rs1;
        iss >> rd >> rs1 >> inst.imm;
        inst.rd = parseReg(rd);
        inst.rs1 = parseReg(rs1);
    }
    else if (op == "STORE") {
        inst.op = OP_STORE;
        std::string rs2, rs1;
        iss >> rs2 >> rs1 >> inst.imm;
        inst.rs2 = parseReg(rs2);
        inst.rs1 = parseReg(rs1);
    }
    else if (op == "BNEZ") {
        inst.op = OP_BNEZ;
        std::string rs1;
        iss >> rs1 >> inst.imm;
        inst.rs1 = parseReg(rs1);
    }
    else if (op == "HALT") {
        inst.op = OP_HALT;
    }
    else {
        inst.op = OP_NOP;
    }

    return inst;
}

std::vector<Instruction> Parser::parseFile(const std::string& filename) {
    std::cout << "Trying to open: "<< filename << std::endl;
    std::ifstream file(filename);
    std::vector<Instruction> program;

    if (!file.is_open()) {
        std::cerr << "Error opening file\n";
        return program;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        program.push_back(parseLine(line));
    }

    return program;
}