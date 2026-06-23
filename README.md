# Cache Behavior Analysis Tool

A cache simulator that processes real memory access traces and compares simulated cache behavior against actual hardware performance metrics.

## Overview

This project analyzes how CPU caches perform on real programs. By simulating cache behavior and comparing it to actual hardware stats, I can understand:
- Why some algorithms have better cache efficiency than others
- How data structure layout affects cache misses
- What makes a cache-friendly program

## Tech Stack

- **C** — Cache simulator
- **Python** — Analysis and visualization (numpy, matplotlib, pandas)
- **Valgrind/perf** — Real trace collection

## Usage

### Step 1: Generate Memory Trace

Use Valgrind's Lackey tool to capture memory accesses from your program:

```bash
gcc -o your_program your_program.c
valgrind --tool=lackey --trace-mem=yes ./your_program > trace.txt
```

This translates the program into memory instructions I, L, S, and M.

### Step 2: Get Expected Cache Behavior

Run Cachegrind on the same program to get actual hardware cache stats:

```bash
valgrind --tool=cachegrind ./your_program
```

Note the output

### Step 3: Run the Simulator

```bash
gcc -o cache_sim cache.c
./cache_sim trace.txt
```

This processes the memory trace through simulated cache

### Step 4: Compare

Compare your simulated results with Cachegrind's actual numbers to validate your cache model.

---

### Configuration

Customize cache parameters (for example):
```bash
./cache_sim trace.txt -s 4096 -a 4 -b 64
```

- `-s` : Cache size (bytes)
- `-a` : Associativity
- `-b` : Block size (bytes)
