#include "Cache.h"

namespace {
uint32_t indexFor(Address address, uint32_t lines) {
    return (lineBase(address) / WORDS_PER_LINE) % lines;
}
uint32_t tagFor(Address address, uint32_t lines) {
    return (lineBase(address) / WORDS_PER_LINE) / lines;
}
}

Cache::Cache(L2Cache& l2) : l2_(l2) {
    reset();
}

void Cache::reset() {
    for (auto& line : lines_) line = CacheLine{};
    hits_ = misses_ = 0;
}

HierarchyResult Cache::readLine(Address address) {
    HierarchyResult out;
    const uint32_t idx = indexFor(address, L1_NUM_LINES);
    const uint32_t tag = tagFor(address, L1_NUM_LINES);
    auto& line = lines_[idx];

    if (line.valid && line.tag == tag) {
        ++hits_;
        out.hit = true;
        out.l2Hit = true;
        out.line = line.data;
        out.word = line.data.words[wordOffset(address)];
        out.latency = L1_HIT_LATENCY;
        out.message = "L1 hit";
        return out;
    }

    ++misses_;
    auto lower = l2_.readLine(address);
    line.valid = true;
    line.dirty = false;
    line.tag = tag;
    line.data = lower.line;

    out.hit = false;
    out.l2Hit = lower.l2Hit;
    out.line = line.data;
    out.word = line.data.words[wordOffset(address)];
    out.latency = L1_HIT_LATENCY + lower.latency;
    out.message = lower.hit ? "L1 miss, L2 hit" : "L1 miss, L2 miss";
    return out;
}

HierarchyResult Cache::readWord(Address address) {
    return readLine(address);
}

HierarchyResult Cache::writeWord(Address address, Word value) {
    HierarchyResult out;
    const uint32_t idx = indexFor(address, L1_NUM_LINES);
    const uint32_t tag = tagFor(address, L1_NUM_LINES);
    auto& line = lines_[idx];

    if (line.valid && line.tag == tag) {
        ++hits_;
        line.data.words[wordOffset(address)] = value;
        auto lower = l2_.writeWord(address, value);
        out.hit = true;
        out.l2Hit = lower.l2Hit;
        out.word = value;
        out.latency = L1_HIT_LATENCY + lower.latency;
        out.message = "L1 write hit, write-through to L2/memory";
        return out;
    }

    ++misses_;
    auto lower = l2_.writeWord(address, value);
    out.hit = false;
    out.l2Hit = lower.l2Hit;
    out.word = value;
    out.latency = lower.latency;
    out.message = "L1 write miss, no-write-allocate";
    return out;
}

const CacheLine* Cache::getLine(uint32_t index) const {
    if (index >= L1_NUM_LINES) return nullptr;
    return &lines_[index];
}
