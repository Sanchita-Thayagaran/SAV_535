#include "L2Cache.h"

namespace {
uint32_t indexFor(Address address, uint32_t lines) {
    return (lineBase(address) / WORDS_PER_LINE) % lines;
}
uint32_t tagFor(Address address, uint32_t lines) {
    return (lineBase(address) / WORDS_PER_LINE) / lines;
}
}

L2Cache::L2Cache(Memory& memory) : memory_(memory) {
    reset();
}

void L2Cache::reset() {
    for (auto& line : lines_) line = CacheLine{};
    hits_ = misses_ = 0;
}

HierarchyResult L2Cache::readLine(Address address) {
    HierarchyResult out;
    const uint32_t idx = indexFor(address, L2_NUM_LINES);
    const uint32_t tag = tagFor(address, L2_NUM_LINES);
    auto& line = lines_[idx];

    if (line.valid && line.tag == tag) {
        ++hits_;
        out.hit = true;
        out.l2Hit = true;
        out.line = line.data;
        out.word = line.data.words[wordOffset(address)];
        out.latency = L2_HIT_LATENCY;
        out.message = "L2 hit";
        return out;
    }

    ++misses_;
    line.valid = true;
    line.dirty = false;
    line.tag = tag;
    line.data = memory_.peekLine(address);

    out.hit = false;
    out.l2Hit = false;
    out.line = line.data;
    out.word = line.data.words[wordOffset(address)];
    out.latency = L2_HIT_LATENCY + DRAM_LATENCY;
    out.message = "L2 miss, fetched from memory";
    return out;
}

HierarchyResult L2Cache::readWord(Address address) {
    return readLine(address);
}

HierarchyResult L2Cache::writeWord(Address address, Word value) {
    HierarchyResult out;
    const uint32_t idx = indexFor(address, L2_NUM_LINES);
    const uint32_t tag = tagFor(address, L2_NUM_LINES);
    auto& line = lines_[idx];

    if (line.valid && line.tag == tag) {
        ++hits_;
        line.data.words[wordOffset(address)] = value;
        memory_.pokeWord(address, value);
        out.hit = true;
        out.l2Hit = true;
        out.latency = L2_HIT_LATENCY + DRAM_LATENCY;
        out.word = value;
        out.message = "L2 write hit, write-through to memory";
        return out;
    }

    ++misses_;
    memory_.pokeWord(address, value);
    out.hit = false;
    out.l2Hit = false;
    out.latency = DRAM_LATENCY;
    out.word = value;
    out.message = "L2 write miss, no-write-allocate to memory";
    return out;
}

const CacheLine* L2Cache::getLine(uint32_t index) const {
    if (index >= L2_NUM_LINES) return nullptr;
    return &lines_[index];
}
