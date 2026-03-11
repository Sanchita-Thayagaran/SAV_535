#pragma once
#include "Types.h"
#include <vector>

class Memory
{
public:
  Memory();

  void reset();

  MemoryResponse readLine(Address address, Stage stage);
  WriteResponse writeWord(Address address, Word value, Stage stage);

  LineData viewLine(uint32_t lineNumber) const;
  Word peekWord(Address address) const;

  bool busy() const;
  Stage activeStage() const;
  int remainingCycles() const;

private:
  Address normalize(Address addr) const;
  uint32_t lineNumberFromAddress(Address addr) const;
  uint32_t numLines() const;

  std::vector<Word> ram_;
  PendingMemoryRequest pending_;
};