#include "Simulator.h"
#include "UI.h"
#include <algorithm>
#include <climits>
#include <sstream>
#include <stdexcept>

namespace {
int32_t add32(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b));
}
int32_t sub32(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b));
}
int32_t mul32(int32_t a, int32_t b) {
    int64_t wide = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(static_cast<uint32_t>(wide));
}
int32_t div32(int32_t a, int32_t b) {
    if (b == 0) return 0;
    if (a == INT32_MIN && b == -1) return INT32_MIN;
    return a / b;
}
int32_t mod32(int32_t a, int32_t b) {
    if (b == 0) return 0;
    if (a == INT32_MIN && b == -1) return 0;
    return a % b;
}
int32_t sll32(int32_t a, int32_t sh) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) << (sh & 0x1F));
}
int32_t srl32(int32_t a, int32_t sh) {
    return static_cast<int32_t>(static_cast<uint32_t>(a) >> (sh & 0x1F));
}
int32_t sra32(int32_t a, int32_t sh) {
    return static_cast<int32_t>(a >> (sh & 0x1F));
}
}

Simulator::Simulator() : l2_(memory_), l1_(l2_) {
    reset();
}

void Simulator::reset() {
    memory_.reset();
    l2_.reset();
    l1_.reset();
    regs_.fill(0);
    status_ = {};
    cycles_ = 0;
    pc_ = 0;
    halted_ = false;
    programBase_ = 0;
    programSize_ = 0;
    IF_ = {};
    ID_ = {};
    EX_ = {};
    MEM_ = {};
    WB_ = {};
    hierarchyPort_ = {};
    seq_ = {};
}

std::string Simulator::loadProgramWords(const std::vector<uint32_t>& words) {
    reset();
    programBase_ = 0;
    programSize_ = static_cast<uint32_t>(words.size());
    for (uint32_t i = 0; i < words.size(); ++i) {
        memory_.pokeWord(programBase_ + i, words[i]);
    }
    pc_ = programBase_;
    seq_.pc = programBase_;
    return "Program loaded";
}

std::string Simulator::loadProgramAsm(const std::string& filename) {
    auto assembled = Assembler::assembleFile(filename);
    return loadProgramWords(assembled.words);
}

void Simulator::setMode(ExecMode mode) {
    mode_ = mode;
}

std::string Simulator::modeString() const {
    switch (mode_) {
        case ExecMode::NO_PIPE_NO_CACHE: return "NO_PIPE_NO_CACHE";
        case ExecMode::NO_PIPE_CACHE:    return "NO_PIPE_CACHE";
        case ExecMode::PIPE_NO_CACHE:    return "PIPE_NO_CACHE";
        case ExecMode::PIPE_CACHE:       return "PIPE_CACHE";
    }
    return "UNKNOWN";
}

int32_t Simulator::readReg(int reg) const {
    if (reg <= 0 || reg > 15) return 0;
    return regs_[reg];
}

void Simulator::writeReg(int reg, int32_t value) {
    if (reg == 0) return;
    if (reg >= 0 && reg < 16) regs_[reg] = value;
}

void Simulator::updateZeroFlagIfNeeded(const Instruction& inst, int32_t value) {
    if (inst.updatesZeroFlag()) status_.Z = (value == 0);
}

int32_t Simulator::evalALU(const Instruction& inst, int32_t a, int32_t b) const {
    switch (inst.op) {
        case Opcode::ADD:  return add32(a, b);
        case Opcode::SUB:  return sub32(a, b);
        case Opcode::MUL:  return mul32(a, b);
        case Opcode::DIV:  return div32(a, b);
        case Opcode::MOD:  return mod32(a, b);
        case Opcode::AND:  return static_cast<int32_t>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
        case Opcode::OR:   return static_cast<int32_t>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
        case Opcode::XOR:  return static_cast<int32_t>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
        case Opcode::NOT:  return static_cast<int32_t>(~static_cast<uint32_t>(a));
        case Opcode::SLL:  return sll32(a, b);
        case Opcode::SRL:  return srl32(a, b);
        case Opcode::SRA:  return sra32(a, b);
        case Opcode::SLT:  return (a < b) ? 1 : 0;
        case Opcode::ADDI: return add32(a, inst.imm);
        default:           return 0;
    }
}

bool Simulator::evalBranch(const Instruction& inst, int32_t a, int32_t b) const {
    switch (inst.op) {
        case Opcode::BEQ: return a == b;
        case Opcode::BNE: return a != b;
        case Opcode::BLT: return a < b;
        case Opcode::BGE: return a >= b;
        default: return false;
    }
}

HierarchyResult Simulator::accessReadWord(Address address, bool cachesEnabled) {
    if (cachesEnabled) return l1_.readWord(address);
    HierarchyResult out;
    out.hit = false;
    out.l2Hit = false;
    out.word = memory_.peekWord(address);
    out.latency = DRAM_LATENCY;
    out.message = "Memory read";
    return out;
}

HierarchyResult Simulator::accessWriteWord(Address address, Word value, bool cachesEnabled) {
    if (cachesEnabled) return l1_.writeWord(address, value);
    memory_.pokeWord(address, value);
    HierarchyResult out;
    out.word = value;
    out.latency = DRAM_LATENCY;
    out.message = "Memory write";
    return out;
}

bool Simulator::startHierarchyRequest(Stage owner, HierarchyPort::Kind kind, Address address,
                                      bool cachesEnabled, Word writeValue, int destReg, uint32_t producerPc) {
    if (hierarchyPort_.busy) return false;
    hierarchyPort_.busy = true;
    hierarchyPort_.owner = owner;
    hierarchyPort_.kind = kind;
    hierarchyPort_.address = address;
    hierarchyPort_.writeValue = writeValue;
    hierarchyPort_.destReg = destReg;
    hierarchyPort_.producerPc = producerPc;
    hierarchyPort_.result = (kind == HierarchyPort::Kind::STORE || kind == HierarchyPort::Kind::SEQ_STORE)
        ? accessWriteWord(address, writeValue, cachesEnabled)
        : accessReadWord(address, cachesEnabled);
    hierarchyPort_.remaining = hierarchyPort_.result.latency;
    return true;
}

void Simulator::advanceHierarchy() {
    if (!hierarchyPort_.busy) return;
    if (hierarchyPort_.remaining > 0) --hierarchyPort_.remaining;
    if (hierarchyPort_.remaining != 0) return;

    switch (hierarchyPort_.kind) {
        case HierarchyPort::Kind::FETCH:
            IF_.valid = true;
            IF_.pc = hierarchyPort_.producerPc;
            IF_.raw = hierarchyPort_.result.word;
            IF_.inst = decodeInstruction(hierarchyPort_.result.word);
            break;
        case HierarchyPort::Kind::LOAD:
            MEM_.wbValue = static_cast<int32_t>(hierarchyPort_.result.word);
            break;
        case HierarchyPort::Kind::STORE:
            break;
        case HierarchyPort::Kind::SEQ_FETCH:
            seq_.raw = hierarchyPort_.result.word;
            seq_.inst = decodeInstruction(seq_.raw);
            seq_.phase = SeqState::Phase::EXECUTE;
            break;
        case HierarchyPort::Kind::SEQ_LOAD:
            seq_.wbValue = static_cast<int32_t>(hierarchyPort_.result.word);
            seq_.phase = SeqState::Phase::WRITEBACK;
            break;
        case HierarchyPort::Kind::SEQ_STORE:
            seq_.phase = SeqState::Phase::WRITEBACK;
            break;
        case HierarchyPort::Kind::NONE:
            break;
    }
    hierarchyPort_ = {};
}

bool Simulator::pendingDestInPipe(int reg) const {
    if (reg == 0) return false;
    auto matches = [&](const PipeReg& p) {
        return p.valid && p.inst.writesRegister() && p.inst.rd == reg;
    };
    return matches(EX_) || matches(MEM_);
}

bool Simulator::hasRawHazard(const Instruction& inst) const {
    if (inst.readsRs1() && pendingDestInPipe(inst.rs1)) return true;
    if (inst.readsRs2() && pendingDestInPipe(inst.rs2)) return true;
    if (inst.isStore() && pendingDestInPipe(inst.rd)) return true;
    if (inst.isJump() && pendingDestInPipe(inst.rs1)) return true;
    return false;
}

uint32_t Simulator::fetchInstructionWord(uint32_t address, bool cachesEnabled, bool& stall, std::string& msg) {
    auto res = accessReadWord(address, cachesEnabled);
    stall = false;
    msg = res.message;
    return res.word;
}

Instruction Simulator::fetchAndDecode(uint32_t address, bool cachesEnabled, bool& stall, std::string& msg) {
    return decodeInstruction(fetchInstructionWord(address, cachesEnabled, stall, msg));
}

void Simulator::doWB() {
    if (!WB_.valid) return;
    if (WB_.inst.writesRegister()) {
        writeReg(WB_.inst.op == Opcode::JAL ? 15 : WB_.inst.rd, WB_.wbValue);
        updateZeroFlagIfNeeded(WB_.inst, WB_.wbValue);
    }
    if (WB_.inst.op == Opcode::HALT) {
        halted_ = true;
    }
    WB_ = {};
}

void Simulator::doMEM(bool cachesEnabled, bool& stall) {
    stall = false;
    if (!MEM_.valid) return;

    if (MEM_.inst.isLoad()) {
        if (MEM_.wbValue == 0 && !hierarchyPort_.busy) {
            if (!startHierarchyRequest(Stage::MEM_STAGE, HierarchyPort::Kind::LOAD,
                                       static_cast<Address>(MEM_.memAddr), cachesEnabled,
                                       0, MEM_.inst.rd, MEM_.pc)) {
                stall = true;
                return;
            }
        }
        if (hierarchyPort_.busy && hierarchyPort_.owner == Stage::MEM_STAGE) {
            stall = true;
            return;
        }
        WB_ = MEM_;
        MEM_ = {};
        return;
    }

    if (MEM_.inst.isStore()) {
        if (!hierarchyPort_.busy) {
            if (!startHierarchyRequest(Stage::MEM_STAGE, HierarchyPort::Kind::STORE,
                                       static_cast<Address>(MEM_.memAddr), cachesEnabled,
                                       static_cast<Word>(MEM_.memData), 0, MEM_.pc)) {
                stall = true;
                return;
            }
        }
        if (hierarchyPort_.busy && hierarchyPort_.owner == Stage::MEM_STAGE) {
            stall = true;
            return;
        }
        WB_ = {};
        MEM_ = {};
        return;
    }

    WB_ = MEM_;
    MEM_ = {};
}

void Simulator::doEX(bool& squash, uint32_t& newPC) {
    squash = false;
    newPC = pc_;
    if (!EX_.valid) return;

    auto& inst = EX_.inst;
    const int32_t a = EX_.opA;
    const int32_t b = EX_.opB;

    if (inst.op == Opcode::HALT) {
        MEM_ = EX_;
        EX_ = {};
        return;
    }

    if (inst.isBranch()) {
        if (evalBranch(inst, a, b)) {
            squash = true;
            newPC = EX_.pc + 1 + static_cast<uint32_t>(inst.imm);
        }
        MEM_ = {};
        EX_ = {};
        return;
    }

    if (inst.op == Opcode::J) {
        squash = true;
        newPC = static_cast<uint32_t>(a);
        MEM_ = {};
        EX_ = {};
        return;
    }

    if (inst.op == Opcode::JAL) {
        EX_.wbValue = static_cast<int32_t>(EX_.pc + 1);
        squash = true;
        newPC = static_cast<uint32_t>(a);
        MEM_ = EX_;
        EX_ = {};
        return;
    }

    if (inst.isLoad() || inst.isStore()) {
        EX_.memAddr = add32(a, inst.imm);
        if (inst.isStore()) EX_.memData = readReg(inst.rd);
        MEM_ = EX_;
        EX_ = {};
        return;
    }

    EX_.wbValue = evalALU(inst, a, b);
    MEM_ = EX_;
    EX_ = {};
}

void Simulator::doID(bool& stall) {
    stall = false;
    if (!ID_.valid) return;
    if (hasRawHazard(ID_.inst)) {
        stall = true;
        return;
    }

    ID_.opA = readReg(ID_.inst.rs1);
    ID_.opB = readReg(ID_.inst.rs2);
    EX_ = ID_;
    ID_ = {};
}

void Simulator::doIF(bool cachesEnabled, bool stall) {
    if (stall || halted_ || IF_.valid || hierarchyPort_.busy) return;
    startHierarchyRequest(Stage::IF_STAGE, HierarchyPort::Kind::FETCH, pc_, cachesEnabled, 0, 0, pc_);
    if (!hierarchyPort_.busy) return;
}

void Simulator::stepPipelineCycle(bool cachesEnabled) {
    advanceHierarchy();

    doWB();

    bool memStall = false;
    doMEM(cachesEnabled, memStall);
    if (memStall) {
        ++cycles_;
        regs_[0] = 0;
        return;
    }

    bool squash = false;
    uint32_t newPC = pc_;
    doEX(squash, newPC);

    bool idStall = false;
    doID(idStall);

    if (!idStall && ID_.valid == false && IF_.valid) {
        ID_ = IF_;
        IF_ = {};
        if (!squash) ++pc_;
    }

    if (squash) {
        IF_ = {};
        ID_ = {};
        pc_ = newPC;
    }

    doIF(cachesEnabled, idStall);

    ++cycles_;
    regs_[0] = 0;
}

void Simulator::stepSequentialCycle(bool cachesEnabled) {
    advanceHierarchy();

    switch (seq_.phase) {
        case SeqState::Phase::IDLE:
            if (halted_) {
                seq_.phase = SeqState::Phase::HALTED;
                break;
            }
            seq_.pc = pc_;
            if (!hierarchyPort_.busy) {
                startHierarchyRequest(Stage::SEQ_STAGE, HierarchyPort::Kind::SEQ_FETCH, seq_.pc, cachesEnabled);
                seq_.phase = SeqState::Phase::FETCH_WAIT;
            }
            break;

        case SeqState::Phase::FETCH_WAIT:
            break;

        case SeqState::Phase::EXECUTE: {
            auto& inst = seq_.inst;
            if (inst.op == Opcode::HALT) {
                halted_ = true;
                seq_.phase = SeqState::Phase::HALTED;
                break;
            }
            const int32_t a = readReg(inst.rs1);
            const int32_t b = readReg(inst.rs2);

            if (inst.isBranch()) {
                if (evalBranch(inst, a, b)) pc_ = seq_.pc + 1 + static_cast<uint32_t>(inst.imm);
                else pc_ = seq_.pc + 1;
                seq_.phase = SeqState::Phase::IDLE;
                break;
            }

            if (inst.op == Opcode::J) {
                pc_ = static_cast<uint32_t>(a);
                seq_.phase = SeqState::Phase::IDLE;
                break;
            }

            if (inst.op == Opcode::JAL) {
                writeReg(15, static_cast<int32_t>(seq_.pc + 1));
                pc_ = static_cast<uint32_t>(a);
                seq_.phase = SeqState::Phase::IDLE;
                break;
            }

            if (inst.isLoad()) {
                seq_.memAddr = add32(a, inst.imm);
                if (!hierarchyPort_.busy) {
                    startHierarchyRequest(Stage::SEQ_STAGE, HierarchyPort::Kind::SEQ_LOAD,
                                          static_cast<Address>(seq_.memAddr), cachesEnabled, 0, inst.rd, seq_.pc);
                    seq_.phase = SeqState::Phase::MEM_WAIT;
                }
                break;
            }

            if (inst.isStore()) {
                seq_.memAddr = add32(a, inst.imm);
                seq_.memData = readReg(inst.rd);
                if (!hierarchyPort_.busy) {
                    startHierarchyRequest(Stage::SEQ_STAGE, HierarchyPort::Kind::SEQ_STORE,
                                          static_cast<Address>(seq_.memAddr), cachesEnabled,
                                          static_cast<Word>(seq_.memData), 0, seq_.pc);
                    seq_.phase = SeqState::Phase::MEM_WAIT;
                }
                break;
            }

            seq_.wbValue = evalALU(inst, a, b);
            seq_.phase = SeqState::Phase::WRITEBACK;
            break;
        }

        case SeqState::Phase::MEM_WAIT:
            break;

        case SeqState::Phase::WRITEBACK:
            if (seq_.inst.writesRegister()) {
                writeReg(seq_.inst.op == Opcode::JAL ? 15 : seq_.inst.rd, seq_.wbValue);
                updateZeroFlagIfNeeded(seq_.inst, seq_.wbValue);
            }
            pc_ = seq_.pc + 1;
            seq_.phase = SeqState::Phase::IDLE;
            break;

        case SeqState::Phase::HALTED:
            break;
    }

    ++cycles_;
    regs_[0] = 0;
}

std::string Simulator::step() {
    if (halted_ && mode_ != ExecMode::NO_PIPE_NO_CACHE && mode_ != ExecMode::NO_PIPE_CACHE) return "HALTED";
    switch (mode_) {
        case ExecMode::NO_PIPE_NO_CACHE: stepSequentialCycle(false); break;
        case ExecMode::NO_PIPE_CACHE:    stepSequentialCycle(true); break;
        case ExecMode::PIPE_NO_CACHE:    stepPipelineCycle(false); break;
        case ExecMode::PIPE_CACHE:       stepPipelineCycle(true); break;
    }
    return "OK";
}

std::string Simulator::run(uint64_t maxCycles) {
    uint64_t start = cycles_;
    while (!halted_ && (cycles_ - start) < maxCycles) {
        step();
        if (mode_ == ExecMode::NO_PIPE_NO_CACHE || mode_ == ExecMode::NO_PIPE_CACHE) {
            if (seq_.phase == SeqState::Phase::HALTED) break;
        }
    }
    return halted_ ? "HALTED" : "Stopped at cycle limit";
}

std::string Simulator::runUntilBreakpoint(uint64_t maxCycles) {
    uint64_t start = cycles_;
    while (!halted_ && (cycles_ - start) < maxCycles) {
        // Check if current PC is a breakpoint
        if (hasBreakpoint(pc_)) {
            return "Hit breakpoint at PC=" + std::to_string(pc_);
        }
        step();
        if (mode_ == ExecMode::NO_PIPE_NO_CACHE || mode_ == ExecMode::NO_PIPE_CACHE) {
            if (seq_.phase == SeqState::Phase::HALTED) break;
        }
    }
    if (halted_) return "HALTED";
    if ((cycles_ - start) >= maxCycles) return "Stopped at cycle limit";
    return "Run complete";
}

void Simulator::setBreakpoint(uint32_t address) {
    if (!hasBreakpoint(address)) {
        breakpoints_.push_back(address);
    }
}

void Simulator::clearBreakpoint(uint32_t address) {
    breakpoints_.erase(
        std::remove(breakpoints_.begin(), breakpoints_.end(), address),
        breakpoints_.end()
    );
}

void Simulator::clearAllBreakpoints() {
    breakpoints_.clear();
}

bool Simulator::hasBreakpoint(uint32_t address) const {
    return std::find(breakpoints_.begin(), breakpoints_.end(), address) != breakpoints_.end();
}

std::vector<uint32_t> Simulator::getBreakpoints() const {
    return breakpoints_;
}

SimulatorSnapshot Simulator::getSnapshot(uint32_t memStart, uint32_t memLines) const {
    SimulatorSnapshot s;
    s.cycles = cycles_;
    s.pc = pc_;
    s.halted = halted_;
    s.zFlag = status_.Z;
    s.mode = modeString();
    s.regs = regs_;

    auto fmtPipe = [](const PipeReg& p) {
        return p.valid ? ("PC=" + std::to_string(p.pc) + "  " + p.inst.toString()) : std::string("<empty>");
    };

    s.ifStage = fmtPipe(IF_);
    s.idStage = fmtPipe(ID_);
    s.exStage = fmtPipe(EX_);
    s.memStage = fmtPipe(MEM_);
    s.wbStage = fmtPipe(WB_);

    s.l1Hits = l1_.hits();
    s.l1Misses = l1_.misses();
    s.l2Hits = l2_.hits();
    s.l2Misses = l2_.misses();

    for (uint32_t i = 0; i < L1_NUM_LINES; ++i) {
        SnapshotCacheRow row;
        row.index = i;
        const auto* line = l1_.getLine(i);
        if (line) {
            row.valid = line->valid;
            row.dirty = line->dirty;
            row.tag = line->tag;
            row.data = line->data.words;
        }
        s.l1Rows.push_back(row);
    }

    for (uint32_t i = 0; i < L2_NUM_LINES; ++i) {
        SnapshotCacheRow row;
        row.index = i;
        const auto* line = l2_.getLine(i);
        if (line) {
            row.valid = line->valid;
            row.dirty = line->dirty;
            row.tag = line->tag;
            row.data = line->data.words;
        }
        s.l2Rows.push_back(row);
    }

    for (uint32_t i = 0; i < memLines; ++i) {
        MemoryRow row;
        row.baseAddress = memStart + i * WORDS_PER_LINE;
        for (uint32_t j = 0; j < WORDS_PER_LINE; ++j) {
            row.data[j] = memory_.peekWord(row.baseAddress + j);
        }
        s.memoryRows.push_back(row);
    }

    return s;
}

std::string Simulator::dumpRegs() const { return UI::formatRegisters(getSnapshot()); }
std::string Simulator::dumpPipeline() const { return UI::formatPipeline(getSnapshot()); }
std::string Simulator::dumpMemoryRange(Address start, Address count) const {
    return UI::formatMemory(getSnapshot(start, (count + WORDS_PER_LINE - 1) / WORDS_PER_LINE).memoryRows);
}
std::string Simulator::dumpCacheL1() const { return UI::formatCacheRows(getSnapshot().l1Rows, "L1 Cache"); }
std::string Simulator::dumpCacheL2() const { return UI::formatCacheRows(getSnapshot().l2Rows, "L2 Cache"); }
std::string Simulator::handleStatus() const { return UI::formatSnapshot(getSnapshot()); }
