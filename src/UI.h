#pragma once
#include "Simulator.h"
#include <string>

namespace UI {
std::string formatSnapshot(const SimulatorSnapshot& s);
std::string formatRegisters(const SimulatorSnapshot& s);
std::string formatPipeline(const SimulatorSnapshot& s);
std::string formatCacheRows(const std::vector<SnapshotCacheRow>& rows, const std::string& title);
std::string formatMemory(const std::vector<MemoryRow>& rows);
}
