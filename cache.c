#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global Variables
cacheBlock ***cache;
uint32_t num_sets;
uint32_t offset_bits;
uint32_t index_bits;
uint32_t tag_bits;
uint32_t num_levels;

// Prefetch Variables
uint32_t prefetch_distance = 0;
uint64_t prefetch_total    = 0;
uint64_t prefetch_useful   = 0;
uint64_t prefetch_pollution = 0;

// Cache statistics
uint32_t L1_hits = 0;
uint32_t L1_misses = 0;
uint32_t L1_reads = 0;
uint32_t L1_writes = 0;

// Input parameters to control the cache.
uint32_t cache_level;
uint32_t L1_cache_size;
uint32_t L1_cache_associativity;
uint32_t L1_cache_block_size;
uint32_t L2_cache_size;
uint32_t L2_cache_associativity;
uint32_t L2_cache_block_size;

// Store the 'time' each block was accessed for LRU (for later)
uint32_t **accessedTime;
uint32_t **L2AccessedTime;
uint32_t counter = 0;
uint32_t L2Counter = 0;

// Fully associative shadow cache for miss classification
uint32_t fa_num_blocks;
uint32_t *fa_tags;
uint8_t  *fa_valid;
uint32_t *fa_time;

// Cache miss classifications
uint64_t compulsory_misses = 0;
uint64_t conflict_misses = 0;
uint64_t capacity_misses = 0; 

uint8_t seen_blocks[SEEN_TABLE_SIZE];

uint32_t log2Bin(uint32_t n) {
    uint32_t result = 0;

    while (n >>= 1) {
        result++;
    }

    return result;
}

int seen_before(uint64_t block_addr) {
    uint64_t slot = block_addr % SEEN_TABLE_SIZE;
    if (seen_blocks[slot]) return 1;
    seen_blocks[slot] = 1;
    return 0;
}



void init_fa_cache() {
    fa_num_blocks = L1_cache_size / L1_cache_block_size;
    fa_tags  = calloc(fa_num_blocks, sizeof(uint32_t));
    fa_valid = calloc(fa_num_blocks, sizeof(uint8_t));
    fa_time  = calloc(fa_num_blocks, sizeof(uint32_t));
}

void init_cache() {
    uint32_t setNum[2] = {0, 0};
    setNum[0] = (L1_cache_size / (L1_cache_block_size * L1_cache_associativity));
    num_sets    = setNum[0]; //Initially

    /*Calculate memory address bit breakdown*/
    offset_bits = log2Bin(L1_cache_block_size);
    index_bits  = log2Bin(num_sets);
    tag_bits    = 32 - offset_bits - index_bits;

    if (cache_level == 2) {
        setNum[1] = (L2_cache_size / (L2_cache_block_size * L2_cache_associativity));
    }

    /*Allocate memory for each level*/
    cache = malloc(sizeof(cacheBlock**) * cache_level /*size of array of sets * num of levels*/);

    /*Allocate memory for each set*/
    for (int i = 0; i < cache_level; i++) {
        cache[i] = malloc(sizeof(cacheBlock*) * setNum[i]);
    }

    for (int y = 0; y < cache_level; y++) {
        uint32_t associativity = 0;
        if (y == 0) {
            associativity = L1_cache_associativity;
        } else {
            associativity = L2_cache_associativity;
        }

        for (int i=0; i < setNum[y]; i++) {
            cache[y][i] = malloc(sizeof(cacheBlock) * associativity /*size of each block * num of blocks per set*/);
            for (int x=0; x < associativity; x++) {
                cache[y][i][x].valid = false;
                cache[y][i][x].dirty = false;
                cache[y][i][x].prefetched = false;
                cache[y][i][x].used      = false;
                cache[y][i][x].tag = 0;
            }
        }
    }

    /*Allocate memory for LRU storage*/
    accessedTime = malloc(sizeof(uint32_t *) * setNum[0]);
    for (uint32_t i = 0; i < setNum[0]; i++)
        accessedTime[i] = calloc(L1_cache_associativity, sizeof(uint32_t));

    if (cache_level == 2) {
        L2AccessedTime = malloc(sizeof(uint32_t *) * setNum[1]);
        for (uint32_t i = 0; i < setNum[1]; i++)
            L2AccessedTime[i] = calloc(L2_cache_associativity, sizeof(uint32_t));
    }
}


void free_fa_cache() {
    free(fa_tags);
    free(fa_valid);
    free(fa_time);
}

void free_cache() { 
  /*Free memory from the inside out*/
  uint32_t setNum[2] = {0, 0};
  setNum[0] = (L1_cache_size / (L1_cache_block_size * L1_cache_associativity));

  if (cache_level == 2) {
    setNum[1] = (L2_cache_size / (L2_cache_block_size * L2_cache_associativity));
  }

  for (int y=0; y < cache_level; y++) {
    for (int i=0; i < setNum[y]; i++) {
      free(cache[y][i]);
    }
    free(cache[y]);
  }
  free(cache);


  for (int i = 0; i < setNum[0]; i++) {
    free(accessedTime[i]);
  }
  free(accessedTime);

  if (cache_level == 2) {
    for (int i = 0; i < setNum[1]; i++) {
      free(L2AccessedTime[i]);
    }
    free(L2AccessedTime); 
  }

  return;
}


int fa_cache_lookup(uint64_t address) {
    uint32_t tag = address >> offset_bits;

    // Check for hit
    for (uint32_t i = 0; i < fa_num_blocks; i++) {
        if (fa_valid[i] && fa_tags[i] == tag) {
            fa_time[i] = counter;
            return 1; // HIT
        }
    }

    // MISS - find LRU way
    uint32_t lru = 0;
    for (uint32_t i = 1; i < fa_num_blocks; i++) {
        if (fa_time[i] < fa_time[lru])
            lru = i;
    }

    // Load new block
    fa_valid[lru] = true;
    fa_tags[lru]  = tag;
    fa_time[lru]  = counter;

    return 0; // MISS
}



void prefetch_block(uint64_t address) {
    uint32_t index = (address >> offset_bits) & ((1U << index_bits) - 1);
    uint32_t tag   = (address >> (offset_bits + index_bits));

    cacheBlock *searchBlock = cache[0][index];

    // Dont prefetch if already in cache
    for (uint32_t i = 0; i < L1_cache_associativity; i++) {
        if (searchBlock[i].valid && searchBlock[i].tag == tag) {
            return;
        }
    }

    // Find LRU way to evict
    uint32_t lru = 0;
    for (uint32_t i = 1; i < L1_cache_associativity; i++) {
        if (accessedTime[index][i] < accessedTime[index][lru])
            lru = i;
    }

    // Track pollution
    if (searchBlock[lru].valid && searchBlock[lru].prefetched && !searchBlock[lru].used) {
        prefetch_pollution++;
    }

    // WRITE BACK
    if (searchBlock[lru].valid && searchBlock[lru].dirty) {
        write_to_memory((searchBlock[lru].tag << (offset_bits + index_bits)) | (index << offset_bits));
    }

    searchBlock[lru].valid     = true;
    searchBlock[lru].dirty     = false;
    searchBlock[lru].tag       = tag;
    searchBlock[lru].prefetched = true;
    searchBlock[lru].used      = false;
    accessedTime[index][lru]   = counter++;

    prefetch_total++;
}


void write_to_memory(uint32_t address) {
    // This is just a simulation -> no functionality
    // Included for readability
}

void read_from_memory(uint32_t address) {
    // This is just a simulation -> no functionality
    // Included for readability
}


//TODO: MISS CLASSIFICATION
int cache_read(uint64_t address) {
    // Simulate cache read, WRITE-BACK, WRITE-ALLOCATE

    uint32_t index = (address >> offset_bits) & ((1U << index_bits) -1);
    uint32_t tag = (address >> (offset_bits + index_bits));
    
    cacheBlock *searchBlock = cache[0][index];

    int matchingBlock = -1;

    for (int i = 0; i < L1_cache_associativity; i++) {
        if (searchBlock[i].valid && searchBlock[i].tag == tag) {
            matchingBlock = i;
            break;
        }
    }

    // HIT
    if (matchingBlock != -1) {
        //Update LRU policy
        accessedTime[index][matchingBlock] = counter;
        counter = counter + 1;

        // WAS THIS PREFETCHED
        if (searchBlock[matchingBlock].prefetched && !searchBlock[matchingBlock].used) {
            prefetch_useful++;
            searchBlock[matchingBlock].used = true;
        }

        return 1;
    }

    uint64_t block_addr = address >> offset_bits;
    if (!seen_before(block_addr)) {
        compulsory_misses++;
    } else if (fa_cache_lookup(address)) {
        conflict_misses++;
    } else {
        capacity_misses++;
    }

    // FIND LRU WAY (to evict)
    uint32_t j = 0;
    for (uint32_t i = 1; i < L1_cache_associativity; i++) {
        if (accessedTime[index][i] < accessedTime[index][j])
            j = i;
    }

    // WRITE BACK
    if (searchBlock[j].valid && searchBlock[j].dirty) {
        write_to_memory((searchBlock[j].tag << (offset_bits + index_bits)) | (index << offset_bits));
    }

    // LOAD (TEMPORAL LOCALITY)
    read_from_memory(address);

    searchBlock[j].valid = true;
    searchBlock[j].tag = tag;
    searchBlock[j].prefetched = false;
    searchBlock[j].used       = false;
    searchBlock[j].dirty = false;
    accessedTime[index][j] = counter++; // Now most recently used


    // Prefetch next N blocks
    for (uint32_t i = 1; i <= prefetch_distance; i++) {
        prefetch_block(address + i * L1_cache_block_size);
    }

    return 0;
}



int cache_write(uint64_t address) {
    // Simulate cache write, WRITE-BACK, WRITE-ALLOCATE
    uint32_t index = (address >> offset_bits) & ((1U << index_bits) -1);
    uint32_t tag = (address >> (offset_bits + index_bits));
    
    cacheBlock *searchBlock = cache[0][index];

    int matchingBlock = -1;

    for (int i = 0; i < L1_cache_associativity; i++) {
        if (searchBlock[i].valid && searchBlock[i].tag == tag) {
            matchingBlock = i;
            break;
        }
    }

    // HIT
    if (matchingBlock != -1) {

        // UPDATE LRU POLICY
        accessedTime[index][matchingBlock] = counter;
        counter = counter + 1;

        searchBlock[matchingBlock].dirty = true;

        return 1;
    } 

    uint64_t block_addr = address >> offset_bits;
    if (!seen_before(block_addr)) {
        compulsory_misses++;
    } else if (fa_cache_lookup(address)) {
        conflict_misses++;
    } else {
        capacity_misses++;
    }

    // FIND LRU WAY (to evict)
    uint32_t j = 0;
    for (uint32_t i = 1; i < L1_cache_associativity; i++) {
        if (accessedTime[index][i] < accessedTime[index][j])
            j = i;
    }

    // WRITE BACK
    if (searchBlock[j].valid && searchBlock[j].dirty) {
        write_to_memory((searchBlock[j].tag << (offset_bits + index_bits)) | (index << offset_bits));
    }

    // WRITE ALLOCATE
    read_from_memory(address);

    searchBlock[j].valid = true;
    searchBlock[j].tag   = tag;
    searchBlock[j].prefetched = false;
    searchBlock[j].used       = false;
    searchBlock[j].dirty = true;

    accessedTime[index][j] = counter;
    counter = counter + 1;

    return 0;
}