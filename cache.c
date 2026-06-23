#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Cache statistics
uint32_t L1_hits = 0;
uint32_t L1_misses = 0;
uint32_t L1_reads = 0;
uint32_t L1_writes = 0;

// Cache parameters
uint32_t cache_size = 4096;      // Default: 4KB
uint32_t cache_associativity = 1; // Default: direct-mapped
uint32_t cache_block_size = 64;   // Default: 64 byte blocks

// Function declarations
int cache_read(uint32_t address);
int cache_write(uint32_t address);

// Parse command line arguments
void parse_args(int argc, char *argv[]) {
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            cache_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            cache_associativity = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            cache_block_size = atoi(argv[++i]);
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
    int result; // HIT or MISS

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

// Print results
void print_results() {
    uint32_t total_accesses = L1_hits + L1_misses;
    double hit_rate = (total_accesses > 0) ? (100.0 * L1_hits / total_accesses) : 0;

    printf("\n=== Cache Simulation Results ===\n");
    printf("Cache size: %u bytes\n", cache_size);
    printf("Associativity: %u-way\n", cache_associativity);
    printf("Block size: %u bytes\n\n", cache_block_size);
    
    printf("L1 Accesses: %u\n", total_accesses);
    printf("L1 Hits: %u\n", L1_hits);
    printf("L1 Misses: %u\n", L1_misses);
    printf("Hit Rate: %.2f%%\n", hit_rate);
    printf("Miss Rate: %.2f%%\n\n", 100.0 - hit_rate);
    
    printf("Reads: %u\n", L1_reads);
    printf("Writes: %u\n", L1_writes);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <trace_file> [-s size] [-a assoc] [-b block_size]\n", argv[0]);
        return 1;
    }

    parse_args(argc, argv);
    
    printf("Processing trace: %s\n", argv[1]);
    process_trace(argv[1]);
    
    print_results();

    return 0;
}


int cache_read(uint32_t address) {
    // Simulate cache read
    // Return 1 for HIT, 0 for MISS
    return 0;
}

int cache_write(uint32_t address) {
    // Return 1 for HIT, 0 for MISS
    return 0;
}