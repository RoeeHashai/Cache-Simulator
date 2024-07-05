#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define unsigned char as uchar for convenience
typedef unsigned char uchar;

// Structure for a cache line
typedef struct cache_line_s {
    uchar valid;    // Valid bit
    uchar frequency; // Frequency of access for LFU replacement policy
    long int tag;    // Tag to identify block of memory
    uchar *block;   // Pointer to the block of memory
} cache_line_t;

// Structure for the cache
typedef struct cache_s {
    uchar s;          // Number of set index bits
    uchar t;          // Number of tag bits
    uchar b;          // Number of block offset bits
    uchar E;          // Number of lines per set (associativity)
    cache_line_t **cache; // Pointer to an array of cache lines (2D array for sets and lines)
} cache_t;

// Function to initialize the cache
cache_t initialize_cache(uchar s, uchar t, uchar b, uchar E) {
    cache_t cache;
    // Init cache params
    cache.s = s;
    cache.t = t;
    cache.b = b;
    cache.E = E;

    // Allocate memory for every set
    int S = 1 << s; // S = 2^s;
    int B = 1 << b; // B = 2^b;

    cache.cache = (cache_line_t **) malloc(S * sizeof(cache_line_t *));
    if (!cache.cache) {
        cache.s = 0; // indicate there was an error allocating memory
        return cache;
    }
    for (int i = 0; i < S; ++i) {
        cache.cache[i] = (cache_line_t *) malloc(E * sizeof(cache_line_t));
        if (!cache.cache[i]) {
            for (int j = 0; j < i; ++j) {
                free(cache.cache[j]);
            }
            free(cache.cache);
            cache.s = 0;
            return cache;
        }
        for (int j = 0; j < E; ++j) {
            cache.cache[i][j].valid = 0;
            cache.cache[i][j].frequency = 0;
            cache.cache[i][j].tag = 0;
            cache.cache[i][j].block = (uchar *) calloc(B, sizeof(uchar));
            if (!cache.cache[i][j].block) {
                // Free previously allocated blocks in the current row
                for (int k = 0; k < j; ++k) {
                    free(cache.cache[i][k].block);
                }
                // Free previously allocated rows and their blocks - Free the blocks in the row and then free the row itself
                for (int l = 0; l < i; l++) {
                    for (int m = 0; m < E; m++) {
                        free(cache.cache[l][m].block);
                    }
                    free(cache.cache[l]);
                }
                // Free the array of row pointers and the cache structure itself
                free(cache.cache);
                cache.s = 0;
                return cache;
            }
        }
    }
    return cache;
}

// Function to find the least frequently used (LFU) cache line
int find_least_frequent(cache_line_t *set, uchar E) {
    int lfuIndex = 0;
    uchar minFreq = set[0].frequency;

    for (int i = 0; i < E; ++i) {
        if (set[i].frequency < minFreq) {
            minFreq = set[i].frequency;
            lfuIndex = i;
        }
    }
    return lfuIndex;
}

uchar read_byte(cache_t cache, uchar *start, long int off) {
    // Example for the bitwise operations for set
    // off := (010111)(010101)(1001) := (tag)(set)(block)
    // sizes := (6)(6)(4)
    // set = 0101110101011001 >> 4 ====> 010111010101
    //          = 1 << 6 ====> 1000000 - 1 ====> 0111111
    //          => 010111010101
    //          &  000000111111
    //          =  000000010101
    long int set = (off >> cache.b) & ((1 << cache.s) - 1);

    // Example for the bitwise operations for tag
    // off := (010111)(010101)(1001) := (tag)(set)(block)
    // sizes := (6)(6)(4)
    // tag = 0101110101011001 >> 10 ====> 010111
    long int tag = off >> (cache.s + cache.b);

    // Example for the bitwise operations for block
    // off := (010111)(010101)(1001) := (tag)(set)(block)
    // sizes := (6)(6)(4)
    // block = 0101110101011001
    //       & 1 << 4 ====> 10000 - 1 ====> 01111
    //       = 1001
    long int block = off & ((1 << cache.b) - 1);

    // check for cache hit
    for (int i = 0; i < cache.E; ++i) {
        if (cache.cache[set][i].valid && cache.cache[set][i].tag == tag) {
            cache.cache[set][i].frequency++;
            return cache.cache[set][i].block[block];
        }
    }

    // in case of cache miss
    int lineToInsertIndex = 0;
    while (lineToInsertIndex < cache.E && cache.cache[set][lineToInsertIndex].valid) {
        lineToInsertIndex++;
    }
    if (lineToInsertIndex == cache.E) { // there is no empty line - find LFU
        lineToInsertIndex = find_least_frequent(cache.cache[set], cache.E);
    }

    // create a pointer to the line that will be added
    cache_line_t *line = &cache.cache[set][lineToInsertIndex];
    line->valid = 1;
    line->frequency = 1;
    line->tag = tag;

    // copy the entire block size of adjacent to the wanted value from memory into the cache line
    for (int i = 0; i < (1 << cache.b); ++i) {
        line->block[i] = start[off - block + i];
    }

    // return the read line
    return line->block[block];
}

void write_byte(cache_t cache, uchar *start, long int off, uchar new_byte) {
    long int set = (off >> cache.b) & ((1 << cache.s) - 1);
    long int tag = off >> (cache.s + cache.b);
    long int block = off & ((1 << cache.b) - 1);

    // Check for cache hit
    for (int i = 0; i < cache.E; ++i) {
        if (cache.cache[set][i].valid && cache.cache[set][i].tag == tag) {
            // Found the cache line, write the byte, and update frequency
            cache.cache[set][i].block[block] = new_byte;
            cache.cache[set][i].frequency++;
            // Write the byte to main memory
            start[off] = new_byte;
            return; // Exit after writing the byte
        }
    }

    // In case of cache miss
    int lineToInsertIndex = 0;
    while (lineToInsertIndex < cache.E && cache.cache[set][lineToInsertIndex].valid) {
        lineToInsertIndex++;
    }
    if (lineToInsertIndex == cache.E) { // No empty line, find LFU
        lineToInsertIndex = find_least_frequent(cache.cache[set], cache.E);
    }

    cache_line_t *line = &cache.cache[set][lineToInsertIndex];
    line->valid = 1;
    line->frequency = 1;
    line->tag = tag;

    start[off] = new_byte;

    for (int i = 0; i < (1 << cache.b); ++i) {
        line->block[i] = start[off - block + i];
    }
}

void print_cache(cache_t cache) {
    int S = 1 << cache.s;
    int B = 1 << cache.b;
    for (int i = 0; i < S; i++) {
        printf("Set %d\n", i);
        for (int j = 0; j < cache.E; j++) {
            printf("%1d %d 0x%0*lx ", cache.cache[i][j].valid, cache.cache[i][j].frequency, cache.t, cache.cache[i][j].tag);
            for (int k = 0; k < B; k++) {
                printf("%02x ", cache.cache[i][j].block[k]);
            }
            puts(""); // Move to a new line after printing each cache line.
        }
    }
}

int main() {
    int n;
    printf("Size of data: ");
    scanf("%d", &n);
    uchar* mem = malloc(n);
    printf("Input data >> ");
    for (int i = 0; i < n; i++)
        scanf("%hhd", mem + i);

    int s, t, b, E;
    printf("s t b E: ");
    scanf("%d %d %d %d", &s, &t, &b, &E);
    cache_t cache = initialize_cache(s, t, b, E);

    while (1) {
        scanf("%d", &n);
        if (n < 0) break;
        read_byte(cache, mem, n);
    }

    puts("");
    print_cache(cache);

    free(mem);
}
