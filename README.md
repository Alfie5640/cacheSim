# Cache Behavior Analysis Tool
A cache simulator that investigates how prefetching aggressiveness interacts with cache associativity across different memory access patterns.

## Research Question
Can next-line prefetching improve cache hit rate beyond spatial/temporal locality, and at what point does prefetch aggressiveness hurt more than it helps?

## Overview
This project simulates CPU cache behaviour on real programs. A custom C simulator replays Valgrind memory traces through a configurable L1 cache model, measuring:
- Hit rate across access patterns and cache configurations
- Miss classification (compulsory, conflict, capacity) via a shadow fully-associative cache
- Prefetch usefulness and cache pollution across prefetch distances 0–16

## Tech Stack
- **C** — Cache simulator
- **Python** (pandas, matplotlib, seaborn) — Analysis and visualisation
- **Valgrind Lackey** — Memory trace generation
- **Docker** — Cross-platform reproducibility

## Quick Start

### With Docker (Works on Mac, Windows, Linux)
Requires Docker Desktop. No other dependencies needed.
```bash
git clone <your-repo>
cd cacheSim
docker-compose up
```
Results and graphs appear in the `results/` folder.

### Without Docker (Linux only)
Requires gcc, valgrind, python3.
```bash
pip install pandas matplotlib seaborn
./run_sim.sh
```

## Configuration
```bash
./cache_sim clean_trace.txt -s 4096 -a 1 -b 16 -p 4
```
| Flag | Description | Default |
|------|-------------|---------|
| `-s` | Cache size in bytes | 4096 |
| `-a` | Associativity (1=direct-mapped, N=N-way, 256=fully associative) | 1 |
| `-b` | Block size in bytes | 16 |
| `-p` | Prefetch distance (blocks ahead to speculatively load) | 0 |

## Access Patterns
| Pattern    | Description                  |
|------------|------------------------------|
| Sequential | Linear array traversal       |
| Strided    | Every 7th element (stride-7) |
| Random     | Random index access          |

## Output
Running the simulation produces the following in `results/`:
- `results.csv` — full data across all 54 configurations
- `hitrate_workload.png` — hit rate by pattern and associativity
- `prefetch_hitrate.png` — hit rate vs prefetch distance
- `conflict.png` — conflict misses by associativity
- `prefetch_efficiency.png` — prefetch usefulness vs distance
- `pollution.png` — cache pollution vs distance
- `heatmap.png` — correlation matrix of cache metrics
