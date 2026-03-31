#include "UI.h"
#include <sstream>
#include <iostream>
#include <iomanip>

// ANSI Color Codes
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"
#define CYAN        "\033[36m"
#define GREEN       "\033[32m"
#define RED         "\033[31m"
#define YELLOW      "\033[33m"
#define MAGENTA     "\033[35m"
#define BRIGHT_CYAN "\033[96m"
#define BRIGHT_GREEN "\033[92m"
#define BRIGHT_YELLOW "\033[93m"
#define BRIGHT_WHITE "\033[97m"

namespace UI {

// ========== ORIGINAL FUNCTION (MUST KEEP THIS!) ==========

std::string prettyPrint(const std::string& raw) {
    std::ostringstream out;
    std::istringstream in(raw);
    std::string line;

    while (std::getline(in, line)) {
        // Highlight cycle
        if (line.find("Cycle:") != std::string::npos) {
            out << YELLOW << line << RESET << "\n";
        }
        // Highlight stalls
        else if (line.find("STALL") != std::string::npos) {
            out << RED << line << RESET << "\n";
        }
        // Highlight pipeline block
        else if (line.find("PIPELINE") != std::string::npos) {
            out << CYAN << line << RESET << "\n";
        }
        else {
            out << line << "\n";
        }
    }

    return out.str();
}

// ========== NEW ENHANCED FUNCTIONS ==========

std::string formatEventIcon(const std::string& event) {
    if (event.find("STALL: memory") != std::string::npos) {
        return BRIGHT_YELLOW "⚠" RESET;
    } else if (event.find("STALL: RAW") != std::string::npos) {
        return BRIGHT_YELLOW "⚠" RESET;
    } else if (event.find("SQUASH") != std::string::npos) {
        return MAGENTA "🔄" RESET;
    } else {
        return BRIGHT_GREEN "✓" RESET;
    }
}

std::string formatCycleHeader(uint64_t cycle, const std::string& event) {
    std::ostringstream oss;
    
    // Determine color based on event
    std::string eventColor = BRIGHT_GREEN;
    if (event.find("STALL") != std::string::npos) {
        eventColor = BRIGHT_YELLOW;
    } else if (event.find("SQUASH") != std::string::npos) {
        eventColor = MAGENTA;
    }
    
    oss << BOLD << BRIGHT_CYAN << "╔════════════════════════════════════════════════════════════════╗\n";
    oss << "║ " << "CYCLE: " << RESET << BRIGHT_WHITE << std::setw(6) << cycle;
    oss << RESET << BOLD << BRIGHT_CYAN << " │ EVENT: " << RESET << eventColor << std::left << std::setw(30) << event;
    oss << RESET << BOLD << BRIGHT_CYAN << "║\n";
    oss << "╠════════════════════════════════════════════════════════════════╣\n" << RESET;
    
    return oss.str();
}

std::string formatPipeline(const std::string& if_inst, const std::string& id_inst,
                           const std::string& ex_inst, const std::string& mem_inst,
                           const std::string& wb_inst) {
    std::ostringstream oss;
    
    auto formatStage = [](const std::string& name, const std::string& inst) -> std::string {
        std::ostringstream s;
        s << BOLD << CYAN << "[" << name << "]" << RESET << " ";
        if (inst == "-") {
            s << DIM << inst << RESET;
        } else {
            s << inst.substr(0, 35);
        }
        return s.str();
    };
    
    oss << "║  " << formatStage("IF", if_inst) << "\n";
    oss << "║      ↓\n";
    oss << "║  " << formatStage("ID", id_inst) << "\n";
    oss << "║      ↓\n";
    oss << "║  " << formatStage("EX", ex_inst) << "\n";
    oss << "║      ↓\n";
    oss << "║  " << formatStage("MEM", mem_inst) << "\n";
    oss << "║      ↓\n";
    oss << "║  " << formatStage("WB", wb_inst) << "\n";
    
    return oss.str();
}

std::string formatMemoryStatus(bool busy, int remainingCycles) {
    std::ostringstream oss;
    
    if (busy) {
        oss << RED << "● BUSY" << RESET << YELLOW << " (" << remainingCycles << " cycles)";
    } else {
        oss << GREEN << "● IDLE" << RESET;
    }
    
    return oss.str();
}

std::string formatStatusBar(bool memBusy, int remainingCycles, uint32_t pc) {
    std::ostringstream oss;
    
    oss << "║  Memory: " << formatMemoryStatus(memBusy, remainingCycles);
    oss << std::setw(30) << "" << "PC: " << BRIGHT_WHITE << pc << RESET << "\n";
    
    return oss.str();
}

std::string formatCompletionBanner(uint64_t cycles, const std::string& registers) {
    std::ostringstream oss;
    
    oss << "\n" << BOLD << BRIGHT_GREEN;
    oss << "╔════════════════════════════════════════════════════════════════╗\n";
    oss << "║                  ✓ PROGRAM COMPLETED                          ║\n";
    oss << "║════════════════════════════════════════════════════════════════║\n";
    oss << RESET;
    
    oss << "║  Total Cycles: " << BRIGHT_WHITE << std::setw(8) << cycles << RESET << "\n";
    oss << "║  Status: All instructions completed successfully\n";
    oss << "║\n";
    oss << registers;
    
    oss << BOLD << BRIGHT_GREEN << "╚════════════════════════════════════════════════════════════════╝\n" << RESET;
    
    return oss.str();
}

std::string formatProgramStart() {
    std::ostringstream oss;
    
    oss << BOLD << BRIGHT_CYAN;
    oss << "╔════════════════════════════════════════════════════════════════╗\n";
    oss << "║           CS535 5-STAGE PIPELINE SIMULATOR                    ║\n";
    oss << "║           Demo 2: Pipeline Execution Engine                   ║\n";
    oss << "╚════════════════════════════════════════════════════════════════╝\n";
    oss << RESET;
    
    return oss.str();
}

std::string formatProgramEnd() {
    std::ostringstream oss;
    
    oss << "\n" << BOLD << BRIGHT_CYAN;
    oss << "╔════════════════════════════════════════════════════════════════╗\n";
    oss << "║  Simulation Complete - Thank you for using CS535 Simulator    ║\n";
    oss << "╚════════════════════════════════════════════════════════════════╝\n";
    oss << RESET;
    
    return oss.str();
}

void printTitleBanner() {
    std::cout << formatProgramStart();
}

void printHelpMenu() {
    std::cout << "\n" << BOLD << BRIGHT_CYAN;
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                       COMMAND REFERENCE                       ║\n";
    std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
    std::cout << RESET;
    
    std::cout << "  " << YELLOW << "LOADPROG <file>" << RESET << "     Load assembly program\n";
    std::cout << "  " << YELLOW << "STEP" << RESET << "               Execute one cycle\n";
    std::cout << "  " << YELLOW << "RUN" << RESET << "                Run to completion\n";
    std::cout << "  " << YELLOW << "REGS" << RESET << "               Show registers\n";
    std::cout << "  " << YELLOW << "MEMRANGE <s> <c>" << RESET << "  Show memory\n";
    std::cout << "  " << YELLOW << "H" << RESET << "                 Show help\n";
    std::cout << "  " << YELLOW << "Q" << RESET << "                 Quit\n";
    
    std::cout << BOLD << BRIGHT_CYAN;
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
}

} // namespace UI