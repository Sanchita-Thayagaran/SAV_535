#pragma once
#include <string>
#include <cstdint>

namespace UI {
    // ORIGINAL FUNCTION - KEEP THIS!
    std::string prettyPrint(const std::string& rawOutput);
    
    // New enhanced functions
    std::string formatCycleHeader(uint64_t cycle, const std::string& event);
    std::string formatPipeline(const std::string& if_inst, const std::string& id_inst,
                               const std::string& ex_inst, const std::string& mem_inst,
                               const std::string& wb_inst);
    std::string formatStatusBar(bool memBusy, int remainingCycles, uint32_t pc);
    std::string formatMemoryStatus(bool busy, int remainingCycles);
    std::string formatEventIcon(const std::string& event);
    std::string formatCompletionBanner(uint64_t cycles, const std::string& registers);
    std::string formatProgramStart();
    std::string formatProgramEnd();
    
    // Utility functions
    void printTitleBanner();
    void printHelpMenu();
    
    // Statistics structure for tracking
    struct ExecutionStats {
        uint64_t totalCycles = 0;
        uint64_t stallCycles = 0;
        uint64_t squashCycles = 0;
        uint64_t advanceCycles = 0;
        uint64_t totalStalls = 0;
        uint64_t totalSquashes = 0;
        uint64_t instructionsRetired = 0;
    };
}