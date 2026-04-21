#include "Memory.h"
#include <stdexcept>

Memory::Memory() {
    reset();
}

void Memory::reset() {
    ram_.fill(0);
}

Word Memory::peekWord(Address address) const {
    if (address >= RAM_WORDS) throw std::out_of_range("Memory read out of range");
    return ram_[address];
}

void Memory::pokeWord(Address address, Word value) {
    if (address >= RAM_WORDS) throw std::out_of_range("Memory write out of range");
    ram_[address] = value;
}

LineData Memory::peekLine(Address address) const {
    LineData line;
    const Address base = lineBase(address);
    for (uint32_t i = 0; i < WORDS_PER_LINE; ++i) {
        line.words[i] = peekWord(base + i);
    }
    return line;
}

void Memory::pokeLine(Address address, const LineData& line) {
    const Address base = lineBase(address);
    for (uint32_t i = 0; i < WORDS_PER_LINE; ++i) {
        pokeWord(base + i, line.words[i]);
    }
}
