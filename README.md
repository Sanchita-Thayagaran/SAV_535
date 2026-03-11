## Implements

- CLI command driver
- RAM: 32768 words
- Direct-mapped unified L1 cache
- 4-word lines
- 8 cache lines
- Tag / valid / dirty / data view
- Write-through
- No-write-allocate
- Stage-aware memory timing
- One active memory request at a time
- Command count used as cycle count

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```
