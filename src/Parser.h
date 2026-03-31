#pragma once
#include "Instruction.h"
#include <vector>
#include <string>

class Parser {
public:
    static std::vector<Instruction> parseFile(const std::string& filename);
};