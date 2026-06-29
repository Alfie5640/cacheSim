#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define SEEN_TABLE_SIZE 1048576

typedef struct {
    uint8_t  valid;
    uint8_t  dirty;
    uint8_t  prefetched;
    uint8_t  used;
    uint32_t tag;
} cacheBlock;

typedef struct {
    uint32_t cache_size;
    uint32_t associativity;
    uint32_t block_size;
    uint32_t num_sets;
    uint32_t offset_bits;
    uint32_t index_bits;
    uint32_t tag_bits;
} cacheConfig;

typedef struct {
    uint32_t hits;
    uint32_t misses;
    uint32_t reads;
    uint32_t writes;
    uint32_t compulsory_misses;
    uint32_t conflict_misses; 
    uint32_t capacity_misses;
} cacheStats;

typedef struct {
    uint32_t distance;
    uint64_t total;
    uint64_t useful;
    uint64_t pollution;
} prefetchStats;

typedef struct {
    uint32_t  num_blocks;
    uint32_t *tags;
    uint8_t  *valid;
    uint32_t *time;
} shadowCache;

typedef struct {
    cacheConfig   config;
    cacheStats    stats;
    prefetchStats prefetch;
    shadowCache   shadow;
    cacheBlock  **blocks;       
    uint32_t    **accessed_time;
    uint32_t      counter;
    uint8_t       seen_blocks[SEEN_TABLE_SIZE];
} simContext;

// Function declarations
uint32_t log2Bin(uint32_t n);

void init_sim(simContext *sim, uint32_t cache_size, uint32_t cache_associativity, uint32_t cache_block_size, uint32_t prefetch_distance);
void free_sim(simContext *sim);
void init_shadow_cache(shadowCache *shadow, cacheConfig config);
void free_shadow_cache(shadowCache *shadow);

void read_from_memory(uint32_t address);
void write_to_memory(uint32_t address);

void prefetch_block(simContext *sim, uint64_t address);

void simulate_read(simContext *sim, uint64_t address);
void simulate_write(simContext *sim, uint64_t address);

void simulate_trace(simContext *sim, FILE *trace_file);

void print_stats(simContext sim);
void save_results_csv(simContext sim, FILE *f, const char *workload);

#endif
