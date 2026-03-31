#include "Simulator.h"
#include "Parser.h"
#include <sstream>
#include <iostream>

Simulator::Simulator() : memory_(), cache_(memory_) {
    reset();
}

void Simulator::reset() {
    memory_.reset();
    cache_.reset();

    cycles_ = 0;
    pc_ = 0;
    halted_ = false;
    squash_ = false;
    branchTarget_ = 0;
    memStall_ = false;
    memAccessType_ = AccessType::READ_LINE;

    regs_.fill(0);
    regs_[1] = 0;  // base address
    regs_[2] = 4;  // loop count
    regs_[3] = 1;  // increment

    IF_ = PipeReg{};
    ID_ = PipeReg{};
    EX_ = PipeReg{};
    MEM_ = PipeReg{};
    WB_ = PipeReg{};
}

std::string Simulator::loadProgram(const std::string& filename) {
    program_ = Parser::parseFile(filename);
    pc_ = 0;
    halted_ = false;

    IF_ = PipeReg{};
    ID_ = PipeReg{};
    EX_ = PipeReg{};
    MEM_ = PipeReg{};
    WB_ = PipeReg{};

    regs_.fill(0);
    regs_[1] = 0;
    regs_[2] = 4;
    regs_[3] = 1;

    std::ostringstream oss;
    oss << "Loaded " << program_.size() << " instructions from " << filename;
    return oss.str();
}

bool Simulator::writesRegister(const Instruction& inst) const {
    return inst.op == OP_ADD || inst.op == OP_ADDI || inst.op == OP_LOAD;
}

bool Simulator::isLoad(const Instruction& inst) const {
    return inst.op == OP_LOAD;
}

bool Simulator::isStore(const Instruction& inst) const {
    return inst.op == OP_STORE;
}

bool Simulator::isBranch(const Instruction& inst) const {
    return inst.op == OP_BNEZ;
}

int Simulator::readReg(int reg) const {
    if (reg < 0 || reg >= 16) return 0;
    if (reg == 0) return 0;
    return regs_[reg];
}

bool Simulator::hasRawHazard(const Instruction& inst) const {
    if (!ID_.valid) return false;

    auto dependsOn = [&](int dest) -> bool {
        if (dest <= 0) return false;
        return inst.rs1 == dest || inst.rs2 == dest;
    };

    if (EX_.valid && writesRegister(EX_.inst) && dependsOn(EX_.inst.rd)) return true;
    if (MEM_.valid && writesRegister(MEM_.inst) && dependsOn(MEM_.inst.rd)) return true;

    return false;
}

void Simulator::commitWB() {
    if (!WB_.valid) return;

    const Instruction& inst = WB_.inst;

    if (inst.op == OP_ADD || inst.op == OP_ADDI) {
        if (inst.rd != 0) regs_[inst.rd] = WB_.exResult;
    } else if (inst.op == OP_LOAD) {
        if (inst.rd != 0) regs_[inst.rd] = WB_.memData;
    } else if (inst.op == OP_HALT) {
        halted_ = true;
    }

    regs_[0] = 0;
}

void Simulator::processMEM(bool& stall) {
    stall = false;
    if (!MEM_.valid) return;

    const Instruction& inst = MEM_.inst;

    if (inst.op == OP_LOAD) {
        auto res = cache_.read(static_cast<Address>(MEM_.memAddr), Stage::MEM_STAGE);

        if (res.status == AccessStatus::WAIT) {
            stall = true;
            lastEvent_ = "STALL: memory";
            return;
        }

        if (res.status == AccessStatus::DONE) {
            uint32_t off = (MEM_.memAddr % RAM_WORDS) % WORDS_PER_LINE;
            MEM_.memData = res.line.words[off];
        }
    } else if (inst.op == OP_STORE) {
        auto res = cache_.write(static_cast<Address>(MEM_.memAddr),
                                static_cast<Word>(MEM_.memData),
                                Stage::MEM_STAGE);

        if (res.status == AccessStatus::WAIT) {
            stall = true;
            lastEvent_ = "STALL: memory";
            return;
        }
    }
}

void Simulator::processEX(bool& squash, size_t& newPC) {
    squash = false;
    newPC = pc_;

    if (!EX_.valid) return;

    Instruction& inst = EX_.inst;

    if (inst.op == OP_ADD) {
        EX_.exResult = readReg(inst.rs1) + readReg(inst.rs2);
    } else if (inst.op == OP_ADDI) {
        EX_.exResult = readReg(inst.rs1) + inst.imm;
    } else if (inst.op == OP_LOAD) {
        EX_.memAddr = readReg(inst.rs1) + inst.imm;
    } else if (inst.op == OP_STORE) {
        EX_.memAddr = readReg(inst.rs1) + inst.imm;
        EX_.memData = readReg(inst.rs2);
    } else if (inst.op == OP_BNEZ) {
        if (readReg(inst.rs1) != 0) {
            squash = true;
            newPC = static_cast<size_t>(EX_.pc + 1 + inst.imm);
            lastEvent_ = "SQUASH: taken branch";
        }
    }
}

void Simulator::processID(bool& stall) {
    stall = false;
    if (!ID_.valid) return;

    stall = hasRawHazard(ID_.inst);
    if (stall) {
        lastEvent_ = "STALL: RAW hazard";
    }
}

void Simulator::processIF(bool stall) {
    if (stall || halted_) return;
    if (pc_ >= program_.size()) return;
    if (IF_.valid) return;

    IF_.inst = program_[pc_];
    IF_.valid = true;
    IF_.pc = static_cast<int>(pc_);
    pc_++;
}

std::string Simulator::stageToString(const PipeReg& preg, const std::string& name) const {
    std::ostringstream oss;
    oss << name << ": ";
    if (!preg.valid) oss << "-";
    else oss << preg.inst.toString();
    return oss.str();
}

std::string Simulator::dumpPipeline() const {
    std::ostringstream oss;
    oss << "---------------- PIPELINE ----------------\n";
    oss << stageToString(IF_, "IF") << "\n";
    oss << stageToString(ID_, "ID") << "\n";
    oss << stageToString(EX_, "EX") << "\n";
    oss << stageToString(MEM_, "MEM") << "\n";
    oss << stageToString(WB_, "WB") << "\n";
    oss << "------------------------------------------";
    return oss.str();
}

std::string Simulator::dumpRegs() const {
    std::ostringstream oss;
    oss << "Registers:\n";
    for (int i = 0; i < 16; ++i) {
        oss << "R" << i << " = " << regs_[i] << "\n";
    }
    return oss.str();
}

std::string Simulator::dumpMemoryRange(Address start, Address count) const {
    std::ostringstream oss;
    for (Address a = start; a < start + count; ++a) {
        oss << "Mem[" << a << "] = " << memory_.peekWord(a) << "\n";
    }
    return oss.str();
}

std::string Simulator::step() {
    cycles_++;
    lastEvent_ = "Advance";

    // 1. commit oldest stage
    commitWB();

    // 2. process current MEM and EX and ID
    bool memStall = false;
    bool rawStall = false;
    bool squash = false;
    size_t newPC = pc_;

    processMEM(memStall);
    if (!memStall) {
        processEX(squash, newPC);
        processID(rawStall);
    }

    // 3. shift pipeline
    if (memStall) {
    // keep MEM on the same instruction so the same request is retried next cycle
    // do not shift IF/ID/EX/MEM forward
    WB_ = PipeReg{};

    std::ostringstream oss;
    oss << "========================================\n";
    oss << "Cycle: " << cycles_ << "\n";
    oss << "Event: " << lastEvent_ << "\n";
    oss << dumpPipeline() << "\n";
    oss << "Memory: " << (memory_.busy() ? "BUSY" : "IDLE")
        << " | Remaining cycles: " << memory_.remainingCycles() << "\n";
    oss << "PC: " << pc_ << "\n";
    oss << "========================================";
    return oss.str();
  } else if (squash) {
        WB_ = MEM_;
        MEM_ = EX_;
        EX_ = PipeReg{};
        ID_ = PipeReg{};
        IF_ = PipeReg{};
        pc_ = newPC;
    } else if (rawStall) {
        WB_ = MEM_;
        MEM_ = EX_;
        EX_ = PipeReg{}; // bubble
        // ID and IF stay
    } else {
        WB_ = MEM_;
        MEM_ = EX_;
        EX_ = ID_;
        ID_ = IF_;
        IF_ = PipeReg{};
        processIF(false);
    }

    std::ostringstream oss;
    oss << "========================================\n";
    oss << "Cycle: " << cycles_ << "\n";
    oss << "Event: " << lastEvent_ << "\n";
    oss << dumpPipeline() << "\n";
    oss << "Memory: " << (memory_.busy() ? "BUSY" : "IDLE")
        << " | Remaining cycles: " << memory_.remainingCycles() << "\n";
    oss << "PC: " << pc_ << "\n";
    oss << "========================================";
    return oss.str();
}

std::string Simulator::run() {
  if (program_.empty()){
    return "No Program Loaded.";
  }
    std::ostringstream oss;

    while (!halted_ || IF_.valid || ID_.valid || EX_.valid || MEM_.valid || WB_.valid || pc_ < program_.size()) {
        oss << step() << "\n";
        if (halted_ && !IF_.valid && !ID_.valid && !EX_.valid && !MEM_.valid && !WB_.valid && pc_ >= program_.size()) {
            break;
        }
    }

    oss << "Program finished.\n";
    oss << dumpRegs();
    return oss.str();
}

// ----------------------
// Demo 1 manual commands
// ----------------------

std::string Simulator::handleRead(Address address, Stage stage) {
    if (stage == Stage::NONE) {
        return "ERROR: stage must be IF or MEM";
    }

    cycles_++;
    auto res = cache_.read(address, stage);

    std::ostringstream oss;
    oss << "[cycle " << cycles_ << "] "
        << "R " << address << " " << ::stageToString(stage) << " -> ";

    if (res.status == AccessStatus::WAIT) {
        oss << "wait";
    } else if (res.status == AccessStatus::DONE) {
        uint32_t off = (address % RAM_WORDS) % WORDS_PER_LINE;
        oss << "done, line=" << lineToHexString(res.line)
            << ", requested_word=" << wordToHex(res.line.words[off]);
    } else {
        oss << "error";
    }

    oss << " | " << res.message;
    return oss.str();
}

std::string Simulator::handleWrite(Word value, Address address, Stage stage) {
    if (stage == Stage::NONE) {
        return "ERROR: stage must be IF or MEM";
    }

    cycles_++;
    auto res = cache_.write(address, value, stage);

    std::ostringstream oss;
    oss << "[cycle " << cycles_ << "] "
        << "W " << wordToHex(value) << " " << address << " " << ::stageToString(stage) << " -> ";

    if (res.status == AccessStatus::WAIT) {
        oss << "wait";
    } else if (res.status == AccessStatus::DONE) {
        oss << "done";
    } else {
        oss << "error";
    }

    oss << " | " << res.message;
    return oss.str();
}

std::string Simulator::handleView(int level, uint32_t line) const {
    std::ostringstream oss;

    if (level == 0) {
        auto ramLine = memory_.viewLine(line);
        oss << "V 0 " << line << " -> RAM line " << line
            << " = " << lineToHexString(ramLine);
        return oss.str();
    }

    if (level == 1) {
        const auto& c = cache_.viewLine(line);

        LineData cacheLineData;
        cacheLineData.words = c.data;

        oss << "V 1 " << line << " -> CACHE line " << (line % CACHE_LINES)
            << " | tag=" << wordToHex(static_cast<Word>(c.tag))
            << " valid=" << (c.valid ? 1 : 0)
            << " dirty=" << (c.dirty ? 1 : 0)
            << " data=" << lineToHexString(cacheLineData);
        return oss.str();
    }

    return "ERROR: level must be 0 (RAM) or 1 (cache)";
}

std::string Simulator::handleHelp() const {
    return "Commands:\n"
           "  R <address> <stage>         e.g. R 12 IF\n"
           "  W <value> <address> <stage> e.g. W 99 12 MEM\n"
           "  V <level> <line>            level 0=RAM, 1=cache\n"
           "  S                           show status\n"
           "  LOADPROG <file>             load program file\n"
           "  STEP                        execute one clock cycle\n"
           "  RUN                         run to completion\n"
           "  REGS                        show registers\n"
           "  MEMRANGE <start> <count>    show memory range\n"
           "  H                           help\n"
           "  Q                           quit";
}

std::string Simulator::handleStatus() const {
    std::ostringstream oss;
    oss << "Status:\n"
        << "  cycle_count=" << cycles_ << "\n"
        << "  memory_busy=" << (memory_.busy() ? "YES" : "NO") << "\n"
        << "  memory_active_stage=" << ::stageToString(memory_.activeStage()) << "\n"
        << "  memory_remaining=" << memory_.remainingCycles() << "\n"
        << "  cache_hits=" << cache_.hits() << "\n"
        << "  cache_misses=" << cache_.misses() << "\n"
        << "  cache_reads=" << cache_.reads() << "\n"
        << "  cache_writes=" << cache_.writes() << "\n"
        << dumpPipeline();
    return oss.str();
}