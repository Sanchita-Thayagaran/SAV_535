#include "Simulator.h"
#include "UI.h"
#include <iostream>
#include <sstream>

int main() {
    Simulator sim;
    std::cout << "SAV_535 Simulator - Interactive CLI\n";
    std::cout << "====================================\n";
    std::cout << "Type HELP for command list\n\n";

    std::string line;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd.empty()) continue;

        if (cmd == "QUIT" || cmd == "EXIT") break;
        else if (cmd == "HELP") {
            std::cout << "\n=== SIMULATOR COMMANDS ===\n";
            std::cout << "Program Control:\n";
            std::cout << "  LOADASM <file>      Load assembly program from file\n";
            std::cout << "  RESET               Reset simulator to initial state\n";
            std::cout << "  MODE <0-3>          Set execution mode:\n";
            std::cout << "                        0 = No Pipeline / No Cache\n";
            std::cout << "                        1 = Cache Only\n";
            std::cout << "                        2 = Pipeline Only\n";
            std::cout << "                        3 = Pipeline + Cache\n\n";
            
            std::cout << "Execution:\n";
            std::cout << "  STEP                Execute one cycle\n";
            std::cout << "  RUN [cycles]        Run until halt or cycle limit (default 500000)\n";
            std::cout << "  RUNBP [cycles]      Run until breakpoint hit\n\n";
            
            std::cout << "Breakpoints:\n";
            std::cout << "  BP <address>        Set breakpoint at address\n";
            std::cout << "  CLEARBP <address>   Clear specific breakpoint\n";
            std::cout << "  LISTBP              List all active breakpoints\n";
            std::cout << "  CLEARBPS            Clear all breakpoints\n\n";
            
            std::cout << "State Inspection:\n";
            std::cout << "  REGS                Display all registers\n";
            std::cout << "  PIPE                Display pipeline state\n";
            std::cout << "  CACHE1              Display L1 cache contents\n";
            std::cout << "  CACHE2              Display L2 cache contents\n";
            std::cout << "  MEM <start> <cnt>   Display memory range\n";
            std::cout << "  STATE               Display complete simulator state\n\n";
            
            std::cout << "Other:\n";
            std::cout << "  HELP                Show this help message\n";
            std::cout << "  QUIT                Exit simulator\n";
            std::cout << "==========================\n\n";
        } 
        else if (cmd == "LOADASM") {
            std::string file;
            iss >> file;
            try {
                std::cout << sim.loadProgramAsm(file) << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
        } else if (cmd == "MODE") {
            int m = 3;
            iss >> m;
            sim.setMode(static_cast<ExecMode>(m));
            std::cout << "Mode set\n";
        } else if (cmd == "STEP") {
            std::cout << sim.step() << "\n";
            std::cout << sim.handleStatus();
        } else if (cmd == "RUN") {
            uint64_t n = 500000;
            iss >> n;
            std::cout << sim.run(n) << "\n";
            std::cout << sim.handleStatus();
        } else if (cmd == "RUNBP") {
            uint64_t n = 500000;
            iss >> n;
            std::cout << sim.runUntilBreakpoint(n) << "\n";
            std::cout << sim.handleStatus();
        } else if (cmd == "BP") {
            uint32_t addr = 0;
            iss >> addr;
            sim.setBreakpoint(addr);
            std::cout << "✓ Breakpoint set at address " << addr << "\n";
        } else if (cmd == "CLEARBP") {
            uint32_t addr = 0;
            iss >> addr;
            sim.clearBreakpoint(addr);
            std::cout << "✓ Breakpoint cleared at address " << addr << "\n";
        } else if (cmd == "LISTBP") {
            auto bps = sim.getBreakpoints();
            if (bps.empty()) {
                std::cout << "No breakpoints set\n";
            } else {
                std::cout << "Active Breakpoints:\n";
                for (auto addr : bps) {
                    std::cout << "  • Address " << addr << "\n";
                }
            }
        } else if (cmd == "CLEARBPS") {
            sim.clearAllBreakpoints();
            std::cout << "✓ All breakpoints cleared\n";
        }

         else if (cmd == "REGS") {
            std::cout << sim.dumpRegs();
        } else if (cmd == "PIPE") {
            std::cout << sim.dumpPipeline();
        } else if (cmd == "CACHE1") {
            std::cout << sim.dumpCacheL1();
        } else if (cmd == "CACHE2") {
            std::cout << sim.dumpCacheL2();
        } else if (cmd == "MEM") {
            uint32_t start = 0, count = 16;
            iss >> start >> count;
            std::cout << sim.dumpMemoryRange(start, count);
        } else if (cmd == "STATE") {
            std::cout << sim.handleStatus();
        } else if (cmd == "RESET") {
            sim.reset();
            std::cout << "Reset\n";
        } else {
            std::cout << "Unknown command\n";
        }
    }
    return 0;
}
