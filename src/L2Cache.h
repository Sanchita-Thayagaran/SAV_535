#pragma once
#include "Memory.h"
#include "Types.h"
#include <array>

class L2Cache {
public:
    explicit L2Cache(Memory& memory);

    void reset();

    HierarchyResult readWord(Address address);
    HierarchyResult readLine(Address address);
    HierarchyResult writeWord(Address address, Word value);

    const CacheLine* getLine(uint32_t index) const;

    uint64_t hits() const { return hits_; }
    uint64_t misses() const { return misses_; }

private:
    Memory& memory_;
    std::array<CacheLine, L2_NUM_LINES> lines_{};
    uint64_t hits_ = 0;
    uint64_t misses_ = 0;
};
