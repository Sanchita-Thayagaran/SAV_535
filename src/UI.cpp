#include "UI.h"
#include "Types.h"
#include <iomanip>
#include <sstream>

namespace UI {

std::string formatRegisters(const SimulatorSnapshot& s) {
    std::ostringstream oss;
    oss << "PC=" << s.pc << "  Cycles=" << s.cycles
        << "  Halted=" << (s.halted ? "yes" : "no")
        << "  Z=" << (s.zFlag ? "1" : "0")
        << "  Mode=" << s.mode << "\n";
    for (int i = 0; i < 16; ++i) {
        oss << "R" << std::setw(2) << std::setfill('0') << i << "="
            << std::setw(11) << std::setfill(' ') << s.regs[i];
        if (i % 4 == 3) oss << "\n";
        else oss << "   ";
    }
    return oss.str();
}

std::string formatPipeline(const SimulatorSnapshot& s) {
    std::ostringstream oss;
    oss << "IF : " << s.ifStage << "\n";
    oss << "ID : " << s.idStage << "\n";
    oss << "EX : " << s.exStage << "\n";
    oss << "MEM: " << s.memStage << "\n";
    oss << "WB : " << s.wbStage << "\n";
    return oss.str();
}

std::string formatCacheRows(const std::vector<SnapshotCacheRow>& rows, const std::string& title) {
    std::ostringstream oss;
    oss << title << "\n";
    for (const auto& r : rows) {
        oss << "[" << r.index << "] "
            << "V=" << r.valid << " D=" << r.dirty
            << " Tag=" << r.tag << "  ";
        for (auto w : r.data) oss << hex32(w) << " ";
        oss << "\n";
    }
    return oss.str();
}

std::string formatMemory(const std::vector<MemoryRow>& rows) {
    std::ostringstream oss;
    oss << "Memory\n";
    for (const auto& row : rows) {
        oss << "[" << row.baseAddress << "] ";
        for (auto w : row.data) oss << hex32(w) << " ";
        oss << "\n";
    }
    return oss.str();
}

std::string formatSnapshot(const SimulatorSnapshot& s) {
    std::ostringstream oss;
    oss << formatRegisters(s) << "\n";
    oss << formatPipeline(s) << "\n";
    oss << "L1 hits=" << s.l1Hits << " misses=" << s.l1Misses
        << " | L2 hits=" << s.l2Hits << " misses=" << s.l2Misses << "\n";
    return oss.str();
}

}
