#pragma once
#include "Assembler.h"
#include "Cache.h"
#include "Instruction.h"
#include "L2Cache.h"
#include "Memory.h"
#include "Types.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

enum class ExecMode {
    NO_PIPE_NO_CACHE,
    NO_PIPE_CACHE,
    PIPE_NO_CACHE,
    PIPE_CACHE
};

struct StatusRegister {
    bool Z = false;
};

struct PipeReg {
    bool valid = false;
    uint32_t pc = 0;
    uint32_t raw = 0;
    Instruction inst{};
    int32_t opA = 0;
    int32_t opB = 0;
    int32_t exResult = 0;
    int32_t memAddr = 0;
    int32_t memData = 0;
    int32_t wbValue = 0;
};

struct SimulatorSnapshot {
    uint64_t cycles = 0;
    uint32_t pc = 0;
    bool halted = false;
    bool zFlag = false;
    std::string mode;
    std::array<int32_t, 16> regs{};

    std::string ifStage;
    std::string idStage;
    std::string exStage;
    std::string memStage;
    std::string wbStage;

    std::vector<SnapshotCacheRow> l1Rows;
    std::vector<SnapshotCacheRow> l2Rows;
    std::vector<MemoryRow> memoryRows;

    uint64_t l1Hits = 0;
    uint64_t l1Misses = 0;
    uint64_t l2Hits = 0;
    uint64_t l2Misses = 0;
};

class Simulator {
public:
    Simulator();

    void reset();
    std::string loadProgramAsm(const std::string& filename);
    std::string loadProgramWords(const std::vector<uint32_t>& words);

    void setMode(ExecMode mode);
    ExecMode mode() const { return mode_; }

    std::string step();
    std::string run(uint64_t maxCycles = 500000);

    SimulatorSnapshot getSnapshot(uint32_t memStart = 0, uint32_t memLines = 16) const;

    std::string dumpRegs() const;
    std::string dumpPipeline() const;
    std::string dumpMemoryRange(Address start, Address count) const;
    std::string dumpCacheL1() const;
    std::string dumpCacheL2() const;
    std::string handleStatus() const;

private:
    Memory memory_;
    L2Cache l2_;
    Cache l1_;

    uint64_t cycles_ = 0;
    uint32_t pc_ = 0;
    bool halted_ = false;
    StatusRegister status_{};
    std::array<int32_t, 16> regs_{};

    uint32_t programBase_ = 0;
    uint32_t programSize_ = 0;
    ExecMode mode_ = ExecMode::PIPE_CACHE;

    PipeReg IF_, ID_, EX_, MEM_, WB_;

    struct HierarchyPort {
        bool busy = false;
        Stage owner = Stage::IF_STAGE;
        uint32_t remaining = 0;
        HierarchyResult result{};
        enum class Kind { NONE, FETCH, LOAD, STORE, SEQ_FETCH, SEQ_LOAD, SEQ_STORE } kind = Kind::NONE;
        Address address = 0;
        Word writeValue = 0;
        uint32_t producerPc = 0;
        int destReg = 0;
    } hierarchyPort_;

    struct SeqState {
        enum class Phase { IDLE, FETCH_WAIT, EXECUTE, MEM_WAIT, WRITEBACK, HALTED } phase = Phase::IDLE;
        uint32_t pc = 0;
        uint32_t raw = 0;
        Instruction inst{};
        int32_t aluResult = 0;
        int32_t memAddr = 0;
        int32_t memData = 0;
        int32_t wbValue = 0;
        int destReg = 0;
        bool branchTaken = false;
        uint32_t branchTarget = 0;
    } seq_;

    uint32_t fetchInstructionWord(uint32_t address, bool cachesEnabled, bool& stall, std::string& msg);
    Instruction fetchAndDecode(uint32_t address, bool cachesEnabled, bool& stall, std::string& msg);

    void stepSequentialCycle(bool cachesEnabled);
    void stepPipelineCycle(bool cachesEnabled);

    HierarchyResult accessReadWord(Address address, bool cachesEnabled);
    HierarchyResult accessWriteWord(Address address, Word value, bool cachesEnabled);
    bool startHierarchyRequest(Stage owner, HierarchyPort::Kind kind, Address address,
                               bool cachesEnabled, Word writeValue = 0, int destReg = 0, uint32_t producerPc = 0);

    void advanceHierarchy();

    void doWB();
    void doMEM(bool cachesEnabled, bool& stall);
    void doEX(bool& squash, uint32_t& newPC);
    void doID(bool& stall);
    void doIF(bool cachesEnabled, bool stall);

    int32_t readReg(int reg) const;
    void writeReg(int reg, int32_t value);
    void updateZeroFlagIfNeeded(const Instruction& inst, int32_t value);

    bool pendingDestInPipe(int reg) const;
    bool hasRawHazard(const Instruction& inst) const;
    std::string modeString() const;

    int32_t evalALU(const Instruction& inst, int32_t a, int32_t b) const;
    bool evalBranch(const Instruction& inst, int32_t a, int32_t b) const;
};
