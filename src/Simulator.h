#pragma once
#include "Cache.h"
#include "Memory.h"
#include <string>

class Simulator
{
public:
  Simulator();

  void reset();

  std::string handleRead(Address address, Stage stage);
  std::string handleWrite(Word value, Address address, Stage stage);
  std::string handleView(int level, uint32_t line) const;
  std::string handleHelp() const;
  std::string handleStatus() const;

private:
  Memory memory_;
  Cache cache_;
  uint64_t cycles_ = 0;
};