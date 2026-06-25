# Cache Behavior Analysis Tool
A cache simulator that processes real memory access traces to analyze cache performance across different access patterns.

## Overview
This project simulates CPU cache behavior on real programs. By replaying memory traces through a configurable cache model, I can see:
- Why some algorithms have better cache efficiency than others
- How data structure layout affects cache misses
- What makes a cache-friendly program

## Tech Stack
- **C** — Cache simulator
- **Valgrind Lackey** — Memory trace generation

## Quick Start
Run the full pipeline for all test patterns in one command:
```bash
./run_sim.sh
```
This compiles everything, generates traces, and runs the simulator for three access patterns: sequential, strided, and random.

## Manual Usage

### Step 1: Generate Memory Trace
```bash
gcc -O0 -g your_program.c -o your_program
valgrind --tool=lackey --trace-mem=yes ./your_program 2> trace.txt
grep -E "^[ ]?[ILSM]" trace.txt > clean_trace.txt
```

### Step 2: Run the Simulator
```bash
gcc -o cache_sim main.c cache.c -lm
./cache_sim clean_trace.txt
```

## Configuration
Customize cache parameters:
```bash
./cache_sim clean_trace.txt -s 4096 -a 1 -b 16
```
- `-s` : Cache size in bytes (default: 4096)
- `-a` : Associativity (default: 1, direct-mapped)
- `-b` : Block size in bytes (default: 16)

## Access Patterns Tested
| Pattern    | Description                  |
|------------|------------------------------|
| Sequential | Linear array traversal       |
| Strided    | Every 7th element (stride-7) |
| Random     | Random index access          |

## Results
> Investigation in progress — results to be updated once access pattern isolation is complete.
