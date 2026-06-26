#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t  valid;
    uint8_t  dirty;
    uint8_t  prefetched;
    uint8_t  used;
    uint32_t tag;
} cacheBlock;

// Global Variables
extern cacheBlock ***cache;
extern uint32_t num_sets;
extern uint32_t offset_bits;
extern uint32_t index_bits;
extern uint32_t tag_bits;
extern uint32_t num_levels;

// Prefetching Variables
extern uint32_t prefetch_distance;
extern uint64_t prefetch_total;
extern uint64_t prefetch_useful;
extern uint64_t prefetch_pollution;

// Cache statistics
extern uint32_t L1_hits;
extern uint32_t L1_misses;
extern uint32_t L1_reads;
extern uint32_t L1_writes;

// Input parameters to control the cache.
extern uint32_t cache_level;
extern uint32_t L1_cache_size;
extern uint32_t L1_cache_associativity;
extern uint32_t L1_cache_block_size;
extern uint32_t L2_cache_size;
extern uint32_t L2_cache_associativity;
extern uint32_t L2_cache_block_size;

// LRU storage
extern uint32_t **accessedTime;
extern uint32_t **L2AccessedTime;
extern uint32_t counter;
extern uint32_t L2Counter;

// Fully-associative shadow cache for miss classification
extern uint32_t fa_num_blocks;
extern uint32_t *fa_tags;
extern uint8_t *fa_valid;
extern uint32_t *fa_time;

void init_fa_cache();
void free_fa_cache();
int fa_cache_lookup(uint64_t address);

// Cache miss classifications
extern uint64_t compulsory_misses;
extern uint64_t conflict_misses; 
extern uint64_t capacity_misses;

#define SEEN_TABLE_SIZE 1048576
extern uint8_t seen_blocks[SEEN_TABLE_SIZE];

// Function declarations
void init_cache();
void free_cache();
void read_from_memory(uint32_t address);
void write_to_memory(uint32_t address);
void prefetch_block(uint64_t address);
uint32_t log2Bin(uint32_t n);
int cache_read(uint64_t address);
int cache_write(uint64_t address);

#endif