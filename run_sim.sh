#!/bin/bash

TESTS="sequential strided random"
ASSOCS="1 4 256"
PREFETCH_DISTANCES="0 1 2 4 8 16"

echo "Compiling cache simulator..."
gcc -o cache_sim main.c cache.c -lm

echo "workload,hits,misses,compulsory,conflict,capacity,reads,writes,hit_rate,prefetch_distance,prefetch_total,prefetch_useful,prefetch_pollution" > results.csv

for test in $TESTS; do
    echo ""
    echo "============================================================"
    echo "WORKLOAD: ${test^^}"
    echo "============================================================"

    echo "Compiling test_${test}.c..."
    gcc -O0 -g test_$test.c -o test_$test

    echo "Generating memory trace..."
    valgrind --tool=lackey --trace-mem=yes ./test_$test 2> trace.txt
    grep -E "^[ ]?[ILSM]" trace.txt > clean.txt

    for assoc in $ASSOCS; do
        echo ""
        echo "************************************************************"
        echo "Associativity: ${assoc}-way"
        echo "************************************************************"

        for pdist in $PREFETCH_DISTANCES; do
            echo ""
            echo "------------------------------------------------------------"
            echo "Configuration"
            echo "  Workload           : $test"
            echo "  Associativity      : ${assoc}-way"
            echo "  Prefetch Distance  : $pdist"
            echo "------------------------------------------------------------"

            export WORKLOAD="${test}_a${assoc}_p${pdist}"
            ./cache_sim clean.txt -a $assoc -p $pdist
        done
    done
done

echo ""
echo "============================================================"
echo "Generating graphs..."
echo "============================================================"

python3 cacheGraphs.py

echo ""
echo "Simulation complete."
echo "Results saved to results.csv"
