#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t log2Bin(uint32_t n) {
    uint32_t result = 0;

    while (n >>= 1) {
        result++;
    }

    return result;
}

int seen_before(simContext* sim, uint64_t block_addr) {
    uint64_t slot = block_addr % SEEN_TABLE_SIZE;
    if (sim->seen_blocks[slot]) return 1;
    sim->seen_blocks[slot] = 1;
    return 0;
}

void init_shadow_cache(shadowCache* shadow, cacheConfig config) {
    shadow->num_blocks = config.cache_size / config.block_size;
    shadow->tags  = calloc(shadow->num_blocks, sizeof(uint32_t));
    shadow->valid = calloc(shadow->num_blocks, sizeof(uint8_t));
    shadow->time  = calloc(shadow->num_blocks, sizeof(uint32_t));
}

void init_sim(simContext* sim, uint32_t cache_size, uint32_t cache_associativity, uint32_t cache_block_size, uint32_t prefetch_distance) {
    sim->config.cache_size = cache_size;
    sim->config.associativity = cache_associativity;
    sim->config.block_size = cache_block_size;
    sim->config.num_sets = (cache_size / (cache_block_size * cache_associativity)); //Initially

    /*Calculate memory address bit breakdown*/
    sim->config.offset_bits = log2Bin(cache_block_size);
    sim->config.index_bits  = log2Bin(sim->config.num_sets);
    sim->config.tag_bits    = 32 - sim->config.offset_bits - sim->config.index_bits;

    /*Allocate memory for cache block array*/
    sim->blocks = malloc(sizeof(cacheBlock*) * sim->config.num_sets); // size of array of sets * num of levels

    /*Allocate memory for each set*/
    for (int i = 0; i < sim->config.num_sets; i++) {
        sim->blocks[i] = calloc(cache_associativity, sizeof(cacheBlock));
    }

    /*Allocate memory for LRU storage*/
    sim->accessed_time = malloc(sizeof(uint32_t *) * sim->config.num_sets);
    for (uint32_t i = 0; i < sim->config.num_sets; i++){
        sim->accessed_time[i] = calloc(cache_associativity, sizeof(uint32_t));
    }
    // zero the stats and counter
    sim->prefetch = (prefetchStats){0};
    sim->prefetch.distance = prefetch_distance;
    sim->stats = (cacheStats){0};
    sim->counter = 0;

    // init the shadow cache
    init_shadow_cache(&sim->shadow, sim->config);
}

void free_shadow_cache(shadowCache* shadow) {
    free(shadow->tags);
    free(shadow->valid);
    free(shadow->time);
}

void free_sim(simContext* sim) { 
  for (int i=0; i < sim->config.num_sets; i++) {
    free(sim->blocks[i]);
  }
  free(sim->blocks);


  for (int i = 0; i < sim->config.num_sets; i++) {
    free(sim->accessed_time[i]);
  }
  free(sim->accessed_time);
  free_shadow_cache(&sim->shadow);
}

int shadow_cache_lookup(simContext* sim, uint64_t address) {
    uint32_t tag = address >> sim->config.offset_bits;

    // Check for hit
    for (uint32_t i = 0; i < sim->shadow.num_blocks; i++) {
        if (sim->shadow.valid[i] && sim->shadow.tags[i] == tag) {
            sim->shadow.time[i] = sim->counter;
            return 1; // HIT
        }
    }

    // MISS - find LRU way
    uint32_t lru = 0;
    for (uint32_t i = 1; i < sim->shadow.num_blocks; i++) {
        if (sim->shadow.time[i] < sim->shadow.time[lru])
            lru = i;
    }

    // Load new block
    sim->shadow.valid[lru] = true;
    sim->shadow.tags[lru]  = tag;
    sim->shadow.time[lru]  = sim->counter;

    return 0; // MISS
}

void prefetch_block(simContext* sim, uint64_t address) {
    uint32_t index = (address >> sim->config.offset_bits) & ((1U << sim->config.index_bits) - 1);
    uint32_t tag   = (address >> (sim->config.offset_bits + sim->config.index_bits));

    cacheBlock *search_block = sim->blocks[index];

    // Dont prefetch if already in cache
    for (uint32_t i = 0; i < sim->config.associativity; i++) {
        if (search_block[i].valid && search_block[i].tag == tag) {
            return;
        }
    }

    // Find LRU way to evict
    uint32_t lru = 0;
    for (uint32_t i = 1; i < sim->config.associativity; i++) {
        if (sim->accessed_time[index][i] < sim->accessed_time[index][lru])
            lru = i;
    }

    // Track pollution
    if (search_block[lru].valid && search_block[lru].prefetched && !search_block[lru].used) {
        sim->prefetch.pollution++;
    }

    // WRITE BACK
    if (search_block[lru].valid && search_block[lru].dirty) {
        write_to_memory((search_block[lru].tag << (sim->config.offset_bits + sim->config.index_bits)) | (index << sim->config.offset_bits));
    }

    search_block[lru].valid           = true;
    search_block[lru].dirty           = false;
    search_block[lru].tag             = tag;
    search_block[lru].prefetched      = true;
    search_block[lru].used            = false;
    sim->accessed_time[index][lru]   = sim->counter++;

    sim->prefetch.total++;
}

void write_to_memory(uint32_t address) {
    // This is just a simulation -> no functionality
    // Included for readability
}

void read_from_memory(uint32_t address) {
    // This is just a simulation -> no functionality
    // Included for readability
}

void simulate_read(simContext* sim, uint64_t address) {
    // Simulate cache read, WRITE-BACK, WRITE-ALLOCATE
    sim->stats.reads++;

    uint32_t index = (address >> sim->config.offset_bits) & ((1U << sim->config.index_bits) -1);
    uint32_t tag = (address >> (sim->config.offset_bits + sim->config.index_bits));
    
    cacheBlock *search_block = sim->blocks[index];

    int matchingBlock = -1;

    for (int i = 0; i < sim->config.associativity; i++) {
        if (search_block[i].valid && search_block[i].tag == tag) {
            matchingBlock = i;
            break;
        }
    }

    // HIT
    if (matchingBlock != -1) {
        //Update LRU policy
        sim->accessed_time[index][matchingBlock] = sim->counter;
        sim->counter++;

        // WAS THIS PREFETCHED
        if (search_block[matchingBlock].prefetched && !search_block[matchingBlock].used) {
            sim->prefetch.useful++;
            search_block[matchingBlock].used = true;
        }

        sim->stats.hits++;
    }

    uint64_t block_addr = address >> sim->config.offset_bits;
    if (!seen_before(sim, block_addr)) {
        sim->stats.compulsory_misses++;
    } else if (shadow_cache_lookup(sim, address)) {
        sim->stats.conflict_misses++;
    } else {
        sim->stats.capacity_misses++;
    }

    // FIND LRU WAY (to evict)
    uint32_t j = 0;
    for (uint32_t i = 1; i < sim->config.associativity; i++) {
        if (sim->accessed_time[index][i] < sim->accessed_time[index][j])
            j = i;
    }

    // WRITE BACK
    if (search_block[j].valid && search_block[j].dirty) {
        write_to_memory((search_block[j].tag << (sim->config.offset_bits + sim->config.index_bits)) | (index << sim->config.offset_bits));
    }

    // LOAD (TEMPORAL LOCALITY)
    read_from_memory(address);

    search_block[j].valid         = true;
    search_block[j].tag           = tag;
    search_block[j].prefetched    = false;
    search_block[j].used          = false;
    search_block[j].dirty         = false;

    sim->accessed_time[index][j] = sim->counter++; // Now most recently used

    // Prefetch next N blocks
    for (uint32_t i = 1; i <= sim->prefetch.distance; i++) {
        prefetch_block(sim, address + i * sim->config.block_size);
    }

    sim->stats.misses++;
}

void simulate_write(simContext* sim, uint64_t address) {
    // Simulate cache write, WRITE-BACK, WRITE-ALLOCATE
    sim->stats.writes++;

    uint32_t index = (address >> sim->config.offset_bits) & ((1U << sim->config.index_bits) -1);
    uint32_t tag = (address >> (sim->config.offset_bits + sim->config.index_bits));
    
    cacheBlock *search_block = sim->blocks[index];

    int matchingBlock = -1;

    for (int i = 0; i < sim->config.associativity; i++) {
        if (search_block[i].valid && search_block[i].tag == tag) {
            matchingBlock = i;
            break;
        }
    }

    // HIT
    if (matchingBlock != -1) {

        // UPDATE LRU POLICY
        sim->accessed_time[index][matchingBlock] = sim->counter;
        sim->counter++;

        search_block[matchingBlock].dirty = true;

        sim->stats.hits++;
    } 

    uint64_t block_addr = address >> sim->config.offset_bits;
    if (!seen_before(sim, block_addr)) {
        sim->stats.compulsory_misses++;
    } else if (shadow_cache_lookup(sim, address)) {
        sim->stats.conflict_misses++;
    } else {
        sim->stats.capacity_misses++;
    }
    // FIND LRU WAY (to evict)
    uint32_t j = 0;
    for (uint32_t i = 1; i < sim->config.associativity; i++) {
        if (sim->accessed_time[index][i] < sim->accessed_time[index][j])
            j = i;
    }

    // WRITE BACK
    if (search_block[j].valid && search_block[j].dirty) {
        write_to_memory((search_block[j].tag << (sim->config.offset_bits + sim->config.index_bits)) | (index << sim->config.offset_bits));
    }

    // WRITE ALLOCATE
    read_from_memory(address);

    search_block[j].valid      = true;
    search_block[j].tag        = tag;
    search_block[j].prefetched = false;
    search_block[j].used       = false;
    search_block[j].dirty      = true;

    sim->accessed_time[index][j] = sim->counter;
    sim->counter++;
    sim->stats.misses++;
}

void simulate_trace(simContext *sim, FILE *trace_file){
    char operation;
    uint64_t address;
    while (fscanf(trace_file, " %c %lx,%*d", &operation, &address) == 2) {
        
        if (operation == 'I') {
            continue; //Not I-cache yet
        } 
        else if (operation == 'L') {
            simulate_read(sim, address);
        } 
        else if (operation == 'S') {
            simulate_write(sim, address);
        } 
        else if (operation == 'M') {
            simulate_read(sim, address);
            simulate_write(sim, address);
        }
    }
}

void print_stats(simContext sim) {
    double hit_rate = 100.0 * sim.stats.hits / (sim.stats.hits + sim.stats.misses);

    printf("L1 Hits:   %u\n", sim.stats.hits);
    printf("L1 Misses: %u\n", sim.stats.misses);
    printf("Compulsory misses: %u\n", sim.stats.compulsory_misses);
    printf("Conflict misses: %u\n", sim.stats.conflict_misses);
    printf("Capacity misses: %u\n", sim.stats.capacity_misses);
    printf("L1 Reads:  %u\n", sim.stats.reads);
    printf("L1 Writes: %u\n", sim.stats.writes);
    printf("Hit rate:  %.2f%%\n", hit_rate);
    if (sim.prefetch.distance > 0) {
        double accuracy = 100.0 * sim.prefetch.useful / sim.prefetch.total;
        double pollution = 100.0 * sim.prefetch.pollution / sim.prefetch.total;
        printf("Prefetch distance: %u\n", sim.prefetch.distance);
        printf("Prefetch total:    %lu\n", sim.prefetch.total);
        printf("Prefetch useful:   %lu (%.2f%%)\n", sim.prefetch.useful, accuracy);
        printf("Prefetch pollution:%lu (%.2f%%)\n", sim.prefetch.pollution, pollution);
    }
}

void save_results_csv(simContext sim, FILE *f, const char *workload){
    double hit_rate = 100.0 * sim.stats.hits / (sim.stats.hits + sim.stats.misses);

    fprintf(f,
        "%s,%u,%u,%u,%u,%u,%u,%u,%.2f,%u,%lu,%lu,%lu\n",
        workload,
        sim.stats.hits,
        sim.stats.misses,
        sim.stats.compulsory_misses,
        sim.stats.conflict_misses,
        sim.stats.capacity_misses,
        sim.stats.reads,
        sim.stats.writes,
        hit_rate,
        sim.prefetch.distance,
        sim.prefetch.total,
        sim.prefetch.useful,
        sim.prefetch.pollution);
}
