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
    uint32_t address;
    int result; // hit or miss

    while (fscanf(fp, " %c %x", &operation, &address) == 2) {
        
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
            
            result = cache_write(address);
            if (result == 1) L1_hits++;
            else L1_misses++;
        }
    }

    fclose(fp);
}

// Print results: TODO AT END
void print_results() {

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <trace_file> [-s size] [-a assoc] [-b block_size]\n", argv[0]);
        return 1;
    }

    parse_args(argc, argv);
    init_cache();
    
    printf("Processing trace: %s\n", argv[1]);
    process_trace(argv[1]);
    
    print_results();

    return 0;
}