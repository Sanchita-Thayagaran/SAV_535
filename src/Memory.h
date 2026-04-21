#pragma once
#include "Types.h"
#include <array>

class Memory {
public:
    Memory();

    void reset();

    Word peekWord(Address address) const;
    void pokeWord(Address address, Word value);

    LineData peekLine(Address address) const;
    void pokeLine(Address address, const LineData& line);

private:
    std::array<Word, RAM_WORDS> ram_{};
};
