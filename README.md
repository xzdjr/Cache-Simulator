# Cache Simulator

A C++ cache hierarchy simulator for CSC3060 Project 4. The simulator models an L1 cache, an optional L2 cache, main-memory latency, configurable replacement policies, and simple hardware prefetchers.

## Features

- Configurable cache size, associativity, block size, and latency
- L1-only mode and L1/L2 hierarchy mode
- Write-back and write-allocate cache behavior
- Replacement policies: `LRU`, `BIP`, `SRRIP`
- Prefetchers: `None`, `NextLine`, `Stride`
- Trace analysis helper for studying access patterns
- Workload generator source for producing personalized traces

## Repository Layout

```text
.
|-- main.cpp                 # Command-line driver
|-- memory_hierarchy.*       # Cache and memory simulation logic
|-- repl_policy.*            # Replacement policy implementations
|-- prefetcher.*             # Prefetcher implementations
|-- interfaces.h             # Shared simulator interfaces
|-- defs.h                   # Shared data structures and constants
|-- Makefile                 # Build and run targets
|-- trace_sanity.txt         # Small sanity-check trace
|-- my_trace.txt             # Personalized experiment trace
|-- trace_analyzer.py        # Trace statistics helper
|-- trace_generator/
|   `-- workload_gen.cpp     # Workload generator source
`-- report.md                # Project report and experiment notes
```

## Requirements

- `g++` with C++11 support
- `make`
- Python 3, only needed for `trace_analyzer.py`

## Build

```bash
make
```

This produces the simulator binary:

```bash
./cache_sim
```

To remove generated build files:

```bash
make clean
```

## Usage

```bash
./cache_sim <trace> <L1_KB> <assoc> <block> <L1_lat> <Mem_lat> \
  [L1_policy] [L1_prefetcher] [--enable-l2 [L2_policy] [L2_prefetcher]]
```

Examples:

```bash
# Task 1: L1 cache connected directly to main memory
make task1

# Task 2: enable the L2 cache
make task2

# Task 3: run the selected best configuration on the personalized trace
make task3
```

Manual runs:

```bash
./cache_sim trace_sanity.txt 32 8 64 1 100
./cache_sim trace_sanity.txt 32 8 64 1 100 --enable-l2
./cache_sim my_trace.txt 32 8 64 1 100 LRU Stride --enable-l2 LRU Stride
```

## Trace Analysis

Use the Python helper to inspect access types, stride behavior, set pressure, and per-window locality:

```bash
python3 trace_analyzer.py my_trace.txt --block-size 64 --l1-kb 32 --assoc 8
```

On Windows, use `python` instead of `python3` if that is how Python is installed.

## Workload Generator

Build the workload generator with:

```bash
make trace_gen
```

Then run:

```bash
./trace_generator/workload_gen
```

## Notes

The detailed implementation summary, testing output, trace observations, and AMAT experiment table are kept in `report.md`.
