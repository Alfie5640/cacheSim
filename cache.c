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

/*Store the 'time' each block was accessed for LRU */
uint32_t **accessedTime;
uint32_t **L2AccessedTime;
uint32_t counter = 0;
uint32_t L2Counter = 0;

uint32_t log2Bin(uint32_t n) {
    uint32_t result = 0;

    while (n >>= 1) {
        result++;
    }

    return result;
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



void write_to_memory(uint32_t address) {
    // This is just a simulation -> no functionality
    // Included for readability
}

void read_from_memory(uint32_t address) {
    // This is just a simulation -> no functionality
    // Included for readability
}



int cache_read(uint64_t address) {
    // Simulate cache read, WRITE-BACK, WRITE-ALLOCATE

    uint32_t index = (address >> offset_bits) & ((1U << index_bits) -1);
    uint32_t tag = (address >> (offset_bits + index_bits));
    
    cacheBlock *searchBlock = &cache[0][index][0];

    if (searchBlock->valid && searchBlock->tag == tag) {
        return 1;
    }

    //WRITE BACK
    if (searchBlock->valid && searchBlock->dirty) {
        write_to_memory((searchBlock->tag << (offset_bits + index_bits)) | (index << offset_bits));
    }

    //TEMPORAL LOCALITY
    // Not found -> put in the cache
    read_from_memory(address);

    searchBlock->valid = true;
    searchBlock->tag = tag;
    searchBlock->dirty = false;

    return 0;
}



int cache_write(uint64_t address) {
    // Simulate cache write, WRITE-BACK, WRITE-ALLOCATE
    uint32_t index = (address >> offset_bits) & ((1U << index_bits) -1);
    uint32_t tag = (address >> (offset_bits + index_bits));
    
    cacheBlock *searchBlock = &cache[0][index][0];

    //WRITE BACK
    if (searchBlock->valid && searchBlock->tag == tag) {
        searchBlock->dirty = true;
        return 1;

    } else {
        // Write back dirty block
        if (searchBlock->valid && searchBlock->dirty) {
            write_to_memory((searchBlock->tag << (offset_bits + index_bits)) | (index << offset_bits));
        }

        // Load block, then write to it
        read_from_memory(address);
        searchBlock->valid = true;
        searchBlock->tag   = tag;
        searchBlock->dirty = true;
        return 0;
    }
}