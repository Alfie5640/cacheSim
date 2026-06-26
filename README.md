# Cache Behavior Analysis Tool
A cache simulator that processes real memory access traces to analyze cache performance across different access patterns and configurations.

## Overview
This project simulates CPU cache behavior on real programs. By replaying memory traces through a configurable cache model, I can understand:
- Why some algorithms have better cache efficiency than others
- How associativity affects conflict misses
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
```bash
./cache_sim clean_trace.txt -s 4096 -a 1 -b 16
```
- `-s` : Cache size in bytes (default: 4096)
- `-a` : Associativity (default: 1, direct-mapped)
- `-b` : Block size in bytes (default: 16)

Special cases:
- `-a 1` — direct-mapped
- `-a N` — N-way set associative
- `-a 256` — fully associative (for default 4KB/16B config)

## Access Patterns Tested
| Pattern    | Description                  |
|------------|------------------------------|
| Sequential | Linear array traversal       |
| Strided    | Every 7th element (stride-7) |
| Random     | Random index access          |

## Results (4KB cache, 16B blocks)

### Direct-mapped (a=1)
| Metric | Value |
|--------|-------|
| Hit Rate | 88.70% |
| Compulsory Misses | 3739 |
| Conflict Misses | 4893 |
| Capacity Misses | 1459 |

### 4-way Set Associative (a=4)
| Metric | Value |
|--------|-------|
| Hit Rate | 93.80% |
| Compulsory Misses | 3739 |
| Conflict Misses | 589 |
| Capacity Misses | 1204 |

### Fully Associative (a=256)
| Metric | Value |
|--------|-------|
| Hit Rate | 94.10% |
| Compulsory Misses | 3739 |
| Conflict Misses | 412 |
| Capacity Misses | 1120 |

### Key Observations
- Compulsory misses are identical across all configurations, first access is always a miss regardless of associativity
- Conflict misses drop from direct-mapped to 4-way (4893 → 589), with diminishing returns beyond that
- Most of the benefit of higher associativity is shown by 4-way, fully associative only improves hit rate by 0.3%
