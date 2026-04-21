#pragma once
#include "Instruction.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct AssembledLine {
    uint32_t address = 0;
    uint32_t word = 0;
    std::string source;
};

struct AssemblyResult {
    std::vector<uint32_t> words;
    std::vector<AssembledLine> listing;
};

class Assembler {
public:
    static AssemblyResult assembleFile(const std::string& filename);
    static AssemblyResult assembleLines(const std::vector<std::string>& lines);

private:
    static std::vector<std::string> readLines(const std::string& filename);
    static std::vector<std::string> preprocess(const std::vector<std::string>& lines);
    static std::unordered_map<std::string, uint32_t> collectLabels(const std::vector<std::string>& lines);

    static Instruction parseInstruction(
        const std::string& line,
        uint32_t pc,
        const std::unordered_map<std::string, uint32_t>& labels
    );

    static int parseRegister(const std::string& token);
    static int parseImm(const std::string& token);
    static int parseAbsoluteOrNumber(const std::string& token,
                                     const std::unordered_map<std::string, uint32_t>& labels);
    static int parseBranchOffset(const std::string& token,
                                 uint32_t pc,
                                 const std::unordered_map<std::string, uint32_t>& labels);
};
