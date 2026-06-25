#!/bin/bash
# run_sim.sh

TESTS="sequential strided random"

# Compile cache sim
gcc -o cache_sim main.c cache.c -lm

for test in $TESTS; do
    echo "=== $test ==="
    
    # Compile test program
    gcc -O0 -g test_$test.c -o test_$test
    
    # Generate and clean trace
    valgrind --tool=lackey --trace-mem=yes ./test_$test 2> trace.txt
    grep -E "^[ ]?[ILSM]" trace.txt > clean.txt
    
    echo "--- SIMULATOR ---"
    ./cache_sim clean.txt
    
    echo ""
done
