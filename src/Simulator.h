#pragma once
#include "Cache.h"
#include "Memory.h"
#include "Instruction.h"
#include <string>
#include <vector>
#include <array>
#include <cstddef>
#include <cstdint>

struct PipeReg {
    Instruction inst;
    bool valid = false;
    int pc = -1;

    int exResult = 0;
    int memAddr = 0;
    int memData = 0;
};

class Simulator {
public:
    Simulator();

    void reset();

    std::string handleRead(Address address, Stage stage);
    std::string handleWrite(Word value, Address address, Stage stage);
    std::string handleView(int level, uint32_t line) const;
    std::string handleHelp() const;
    std::string handleStatus() const;

    std::string loadProgram(const std::string& filename);
    std::string step();
    std::string run();
    std::string dumpRegs() const;
    std::string dumpPipeline() const;
    std::string dumpMemoryRange(Address start, Address count) const;

private:
    Memory memory_;
    Cache cache_;
    uint64_t cycles_ = 0;
    size_t pc_ = 0;

    std::vector<Instruction> program_;
    std::array<int, 16> regs_{};

    PipeReg IF_;
    PipeReg ID_;
    PipeReg EX_;
    PipeReg MEM_;
    PipeReg WB_;

    bool halted_ = false;
    bool squash_ = false;
    size_t branchTarget_ = 0;

    bool memStall_ = false;
    AccessType memAccessType_ = AccessType::READ_LINE;

    bool writesRegister(const Instruction& inst) const;
    bool isLoad(const Instruction& inst) const;
    bool isStore(const Instruction& inst) const;
    bool isBranch(const Instruction& inst) const;
    bool hasRawHazard(const Instruction& inst) const;
    int readReg(int reg) const;

    void commitWB();
    void processMEM(bool& stall);
    void processEX(bool& squash, size_t& newPC);
    void processID(bool& stall);
    void processIF(bool stall);

    std::string stageToString(const PipeReg& preg, const std::string& name) const;
    std::string lastEvent_;
};