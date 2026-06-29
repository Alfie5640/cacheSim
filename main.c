#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <trace_file> [-s size] [-a assoc] [-b block_size] [-p prefetch_distance]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *trace_filename = argv[1];
    uint32_t cache_size        = 4096;
    uint32_t associativity     = 1;
    uint32_t block_size        = 16;
    uint32_t prefetch_distance = 0;

    for (int i = 2; i < argc - 1; i += 2) {
        if (strcmp(argv[i], "-s") == 0) 
            cache_size = atoi(argv[i+1]);
        else if (strcmp(argv[i], "-a") == 0)
            associativity = atoi(argv[i+1]);
        else if (strcmp(argv[i], "-b") == 0) 
            block_size = atoi(argv[i+1]);
        else if (strcmp(argv[i], "-p") == 0)
            prefetch_distance = atoi(argv[i+1]);
    }

    simContext sim = {0};

    init_sim(&sim, cache_size, associativity, block_size, prefetch_distance);

    printf("Processing trace: %s\n", trace_filename);

    FILE *trace_file = fopen(trace_filename, "r");
    if (!trace_file) {
        perror("Error opening trace file");
        return 1;
    }

    simulate_trace(&sim, trace_file);
    fclose(trace_file);

    print_stats(sim);

    char *workload = getenv("WORKLOAD");
    if (workload != NULL) {
        FILE *csv_file = fopen("results/results.csv", "a");

        if (csv_file == NULL) {
            perror("results.csv");
            return 1;
        }
        save_results_csv(sim, csv_file, workload);

        fclose(csv_file);
    }

    free_sim(&sim);
    return 0;
}
