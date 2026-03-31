#include "Simulator.h"
#include <iostream>
#include <sstream>
#include <string>

int main()
{
  Simulator sim;

  std::cout << "CS535 Memory/Cache + Pipeline Simulator\n";
  std::cout << sim.handleHelp() << "\n";
  std::cout << "Initial state: RAM is zero-initialized, cache lines invalid.\n";

  std::string line;
  while (true)
  {
    std::cout << "\n> ";
    if (!std::getline(std::cin, line))
    {
      break;
    }

    if (line.empty())
    {
      continue;
    }

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    // ----------------------
    // Exit / Help
    // ----------------------
    if (cmd == "Q" || cmd == "q")
    {
      std::cout << "Exiting.\n";
      break;
    }

    if (cmd == "H" || cmd == "h")
    {
      std::cout << sim.handleHelp() << "\n";
      continue;
    }

    // ----------------------
    // Status
    // ----------------------
    if (cmd == "S" || cmd == "s")
    {
      std::cout << sim.handleStatus() << "\n";
      continue;
    }

    // ----------------------
    // View (cache / RAM)
    // ----------------------
    if (cmd == "V" || cmd == "v")
    {
      int level;
      uint32_t lineNum;
      if (!(iss >> level >> lineNum))
      {
        std::cout << "ERROR: usage V <level> <line>\n";
        continue;
      }
      std::cout << sim.handleView(level, lineNum) << "\n";
      continue;
    }

    // ----------------------
    // Read
    // ----------------------
    if (cmd == "R" || cmd == "r")
    {
      Address addr;
      std::string stageStr;
      if (!(iss >> addr >> stageStr))
      {
        std::cout << "ERROR: usage R <address> <stage>\n";
        continue;
      }
      std::cout << sim.handleRead(addr, parseStage(stageStr)) << "\n";
      continue;
    }

    // ----------------------
    // Write
    // ----------------------
    if (cmd == "W" || cmd == "w")
    {
      Word value;
      Address addr;
      std::string stageStr;
      if (!(iss >> value >> addr >> stageStr))
      {
        std::cout << "ERROR: usage W <value> <address> <stage>\n";
        continue;
      }
      std::cout << sim.handleWrite(value, addr, parseStage(stageStr)) << "\n";
      continue;
    }

    // =====================================================
    // DEMO 2 (PIPELINE) COMMANDS
    // =====================================================

    if (cmd == "LOADPROG")
    {
      std::string file;
      if (!(iss >> file))
      {
        std::cout << "ERROR: usage LOADPROG <file>\n";
        continue;
      }
      std::cout << sim.loadProgram(file) << "\n";
      continue;
    }

    if (cmd == "STEP")
    {
      std::cout << sim.step() << "\n";
      continue;
    }

    if (cmd == "RUN")
    {
      std::cout << sim.run() << "\n";
      continue;
    }

    if (cmd == "REGS")
    {
      std::cout << sim.dumpRegs() << "\n";
      continue;
    }

    if (cmd == "MEMRANGE")
    {
      Address start, count;
      if (!(iss >> start >> count))
      {
        std::cout << "ERROR: usage MEMRANGE <start> <count>\n";
        continue;
      }
      std::cout << sim.dumpMemoryRange(start, count) << "\n";
      continue;
    }

    // ----------------------
    // Unknown command
    // ----------------------
    std::cout << "ERROR: unknown command. Type H for help.\n";
  }

  return 0;
}