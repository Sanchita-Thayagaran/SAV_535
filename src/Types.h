#pragma once
#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using Word = uint32_t;
using Address = uint32_t;

constexpr uint32_t RAM_WORDS = 32768;
constexpr uint32_t WORDS_PER_LINE = 4;

constexpr uint32_t L1_NUM_LINES = 8;
constexpr uint32_t L2_NUM_LINES = 32;

constexpr uint32_t L1_HIT_LATENCY = 1;
constexpr uint32_t L2_HIT_LATENCY = 10;
constexpr uint32_t DRAM_LATENCY = 50;

enum class Stage
{
    IF_STAGE,
    MEM_STAGE,
    SEQ_STAGE
};

struct LineData
{
    std::array<Word, WORDS_PER_LINE> words{};
};

struct CacheLine
{
    bool valid = false;
    bool dirty = false;
    uint32_t tag = 0;
    LineData data{};
};

struct HierarchyResult
{
    bool hit = false;
    bool l2Hit = false;
    Word word = 0;
    LineData line{};
    uint32_t latency = 0;
    std::string message;
};

struct SnapshotCacheRow
{
    uint32_t index = 0;
    bool valid = false;
    bool dirty = false;
    uint32_t tag = 0;
    std::array<uint32_t, 4> data{};
};

struct MemoryRow
{
    uint32_t baseAddress = 0;
    std::array<uint32_t, 4> data{};
};

inline std::string hex32(uint32_t v)
{
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase
        << std::setw(8) << std::setfill('0') << v;
    return oss.str();
}

inline uint32_t lineBase(Address address)
{
    return address - (address % WORDS_PER_LINE);
}

inline uint32_t wordOffset(Address address)
{
    return address % WORDS_PER_LINE;
}