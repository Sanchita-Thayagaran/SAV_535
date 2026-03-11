#include "Simulator.h"
#include <iostream>
#include <sstream>
#include <string>

int main()
{
  Simulator sim;

  std::cout << "CS535 Memory/Cache Demo\n";
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

    if (cmd == "S" || cmd == "s")
    {
      std::cout << sim.handleStatus() << "\n";
      continue;
    }

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

    std::cout << "ERROR: unknown command. Type H for help.\n";
  }

  return 0;
}