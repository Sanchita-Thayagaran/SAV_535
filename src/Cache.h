#pragma once
#include "Memory.h"
#include "Types.h"
#include <vector>

class Cache
{
public:
  explicit Cache(Memory &memory);

  void reset();

  MemoryResponse read(Address address, Stage stage);
  WriteResponse write(Address address, Word value, Stage stage);

  const CacheLine &viewLine(uint32_t lineIndex) const;

  uint64_t hits() const;
  uint64_t misses() const;
  uint64_t reads() const;
  uint64_t writes() const;

private:
  uint32_t blockNumber(Address addr) const;
  uint32_t lineIndex(Address addr) const;
  uint32_t tag(Address addr) const;
  uint32_t offset(Address addr) const;
  Address blockBaseAddress(Address addr) const;

  Memory &memory_;
  std::vector<CacheLine> lines_;

  uint64_t hits_ = 0;
  uint64_t misses_ = 0;
  uint64_t reads_ = 0;
  uint64_t writes_ = 0;
};