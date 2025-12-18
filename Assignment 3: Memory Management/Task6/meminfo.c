#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num_pages> [initialize]\n", argv[0]);
        fprintf(stderr, "  num_pages: Number of pages to allocate (e.g., 128)\n");
        fprintf(stderr, "  initialize: Optional flag to initialize memory (1 = yes, 0 = no, default = 0)\n");
        fprintf(stderr, "\nExample without initialization:\n");
        fprintf(stderr, "  /usr/bin/time --verbose %s 128\n", argv[0]);
        fprintf(stderr, "\nExample with initialization:\n");
        fprintf(stderr, "  /usr/bin/time --verbose %s 128 1\n", argv[0]);
        return 1;
    }

    // Parse arguments
    int num_pages = atoi(argv[1]);
    int initialize = 0;

    if (argc >= 3) {
        initialize = atoi(argv[2]);
    }

    if (num_pages <= 0) {
        fprintf(stderr, "Error: num_pages must be a positive integer\n");
        return 1;
    }

    // Get system page size
    int page_size = getpagesize();
    size_t total_bytes = (size_t)num_pages * page_size;

    printf("=== Memory Allocation Program ===\n");
    printf("Page size: %d bytes\n", page_size);
    printf("Number of pages: %d\n", num_pages);
    printf("Total memory to allocate: %zu bytes (%.2f MB)\n",
           total_bytes, total_bytes / (1024.0 * 1024.0));
    printf("Initialize memory: %s\n\n", initialize ? "Yes" : "No");

    // Allocate memory
    printf("Allocating memory using malloc()...\n");
    char *memory = (char *)malloc(total_bytes);

    if (memory == NULL) {
        fprintf(stderr, "Error: malloc() failed to allocate %zu bytes\n", total_bytes);
        return 1;
    }

    printf("Memory allocated successfully at address: %p\n", (void *)memory);

    // Optionally initialize memory
    if (initialize) {
        printf("Initializing memory with memset()...\n");
        memset(memory, 0, total_bytes);
        printf("Memory initialized successfully\n");
    } else {
        printf("Memory NOT initialized (just allocated)\n");
    }

    printf("\nMemory allocation complete.\n");
    printf("The program will now exit and release the memory.\n");

    // Free memory
    free(memory);

    return 0;
}
