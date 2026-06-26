#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parse command line arguments
void parse_args(int argc, char *argv[]) {
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            L1_cache_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            L1_cache_associativity = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            L1_cache_block_size = atoi(argv[++i]);
        }
    }
}

// Parse trace file and simulate cache
void process_trace(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening trace file");
        return;
    }

    char operation;
    uint64_t address;
    int result; // hit or miss

    while (fscanf(fp, " %c %lx,%*d", &operation, &address) == 2) {
        
        if (operation == 'I') {
            continue; //Not I-cache yet
        } 
        else if (operation == 'L') {
            // Load -> read
            L1_reads++;
            result = cache_read(address);
            if (result == 1) L1_hits++;
            else L1_misses++;
        } 
        else if (operation == 'S') {
            // Store -> write
            L1_writes++;
            result = cache_write(address);
            if (result == 1) L1_hits++;
            else L1_misses++;
        } 
        else if (operation == 'M') {
            // Modify -> read + write
            L1_reads++;
            result = cache_read(address);
            if (result == 1) L1_hits++;
            else L1_misses++;
            
            L1_writes++;
            result = cache_write(address);
            if (result == 1) L1_hits++;
            else L1_misses++;
        }
    }

    fclose(fp);
}

// Print results
// Print results
void print_results() {
    double hit_rate = 100.0 * L1_hits / (L1_hits + L1_misses);

    printf("L1 Hits:   %u\n", L1_hits);
    printf("L1 Misses: %u\n", L1_misses);
    printf("Compulsory misses: %lu\n", compulsory_misses);
    printf("Conflict misses: %lu\n", conflict_misses);
    printf("Capacity misses: %lu\n", capacity_misses);
    printf("L1 Reads:  %u\n", L1_reads);
    printf("L1 Writes: %u\n", L1_writes);
    printf("Hit rate:  %.2f%%\n", hit_rate);
}

void save_results_csv(const char *workload) {
    FILE *fp = fopen("results.csv", "a");

    if (fp == NULL) {
        perror("results.csv");
        return;
    }

    double hit_rate = 100.0 * L1_hits / (L1_hits + L1_misses);

    fprintf(fp,
        "%s,%u,%u,%lu,%lu,,%lu,%u,%u,%.2f\n",
        workload,
        L1_hits,
        L1_misses,
        compulsory_misses,
        conflict_misses,
        capacity_misses,
        L1_reads,
        L1_writes,
        hit_rate);

    fclose(fp);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <trace_file> [-s size] [-a assoc] [-b block_size]\n", argv[0]);
        return 1;
    }

    //DEFAULT VALUES
    cache_level = 1;
    L1_cache_size = 4096;        // 4KB
    L1_cache_associativity = 1;  // direct-mapped
    L1_cache_block_size = 16;

    parse_args(argc, argv);

    init_cache();
    init_fa_cache();
    
    printf("Processing trace: %s\n", argv[1]);
    process_trace(argv[1]);
    
    print_results();
    char *workload = getenv("WORKLOAD");
    if (workload != NULL) {
        save_results_csv(workload);
    }

    free_cache();
    free_fa_cache();

    return 0;
}