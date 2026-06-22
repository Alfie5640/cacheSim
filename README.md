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
