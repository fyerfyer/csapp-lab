/*
 * Cache Simulator written by fyerfyer
 * 2024.8.12
 */
#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct cache_line {
    int valid;
    int tag;        
    int time_stamp; // if the stamp is larger, then it is the more previous one
} Line;

typedef struct cache {
    int S, E, B;
    Line **lines;
} Cache;

int verbose = 0;   // if it's needed to print detailed info
int hit_count = 0, miss_count = 0, evict_count = 0;
Cache *cache = NULL;
char file[1000];

void init_cache(int s, int E, int b) {
    cache = (Cache*) malloc(sizeof(Cache));
    
    int S = 1 << s, B = 1 << b;
    cache->S = S;
    cache->E = E;
    cache->B = B;
    cache->lines = (Line**) malloc(sizeof(Line *) * S);
    
    if (cache->lines == NULL) 
        exit(-1);

    for(int i = 0; i < S; ++ i) {
        cache->lines[i] = (Line*) malloc(sizeof(Line) * E);
        if (cache->lines[i] == NULL) 
            exit(-1);

        for (int j = 0; j < E; ++ j) {
            cache->lines[i][j].valid = 0;
            cache->lines[i][j].tag = -1;      
            cache->lines[i][j].time_stamp = 0;
        }
    }
}

void free_cache() {
    int S = cache->S;
    for (int i = 0; i < S; ++ i) {
        free(cache->lines[i]);
    }

    free(cache->lines);
    free(cache);
}

int get_index(int op_set, int op_tag) {
    int E = cache->E;
    for (int i = 0; i < E; ++ i) {
        if (cache->lines[op_set][i].valid && cache->lines[op_set][i].tag == op_tag)
            return i;
    }

    return -1;
}

int find_LRU(int op_set) {
    int index = 0;
    int max_stamp = 0;
    int E = cache->E;
    for (int i = 1; i < E; ++ i) {
        if (cache->lines[op_set][i].time_stamp > max_stamp) {
            max_stamp = cache->lines[op_set][i].time_stamp;
            index = i;
        }
    }

    return index;
}

int find_freeplace(int op_set) {
    int E = cache->E;
    for (int i = 0; i < E; ++ i) {
        if (!cache->lines[op_set][i].valid)
            return i;
    }

    return -1;
}

void update_helper(int line_index, int op_set, int op_tag) {
    cache->lines[op_set][line_index].valid = 1;
    cache->lines[op_set][line_index].tag = op_tag;

    // update the time_stamp
    int E = cache->E;
    for (int i = 0; i < E; ++ i) {
        if (cache->lines[op_set][i].valid)
            cache->lines[op_set][i].time_stamp ++;
    }

    cache->lines[op_set][line_index].time_stamp = 0;
}

void update(int op_set, int op_tag) {
    int index = get_index(op_set, op_tag);
    if (index == -1) {
        miss_count ++;
        if (verbose) 
            printf("miss");
        
        int index = find_freeplace(op_set);
        // there's no free place, take the place of LRU
        if (index == -1) {
            evict_count ++;
            if (verbose)
                printf("eviction");
            index = find_LRU(op_set);
        }

        update_helper(index, op_set, op_tag);
    } else {
        hit_count ++;
        if (verbose) 
            printf("hit");
        update_helper(index, op_set, op_tag);
    }
}

void trace(int s, int E, int b) {
    FILE *File;
    File = fopen(file, "r");
    if (File == NULL) {
        printf("No such file!\n");
        exit(-1);
    }

    char type;
    unsigned address;
    int size;

    while (fscanf(File, " %c %x,%d", &type, &address, &size) > 0) {
        int op_tag = address >> (s + b);
        int op_set = (address >> b) & ((unsigned)(-1) >> (32 - s));

        if (type != 'I' && type != 'M' && type != 'L' && type != 'S') {
            printf("Invalid instruction!\n");
            exit(-1);
        }
        if (type == 'I') continue;
        if (type == 'M') 
            update(op_set, op_tag);
        update(op_set, op_tag);
    }
    fclose(File);
}

void print_help() {
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n\n");
}

int main(int argc, char *argv[]) {
    char opt;
    int s, E, b;
    while (-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
        if (opt == 'h') {
            print_help();
            continue;
        }

        if (opt == 'v') {
            verbose = 1;
            continue;
        }

        if (opt == 's') {
            s = atoi(optarg);
            continue;
        }

        if (opt == 'E') {
            E = atoi(optarg);
            continue;
        }

        if (opt == 'b') {
            b = atoi(optarg);
            continue;
        }

        if (opt == 't') {
            strcpy(file, optarg);
            continue;
        } else {
            printf("Invalid input!\n");
            exit(-1);
        }
    }

    init_cache(s, E, b);
    trace(s, E, b);
    free_cache();

    printSummary(hit_count, miss_count, evict_count);
    return 0;
}
