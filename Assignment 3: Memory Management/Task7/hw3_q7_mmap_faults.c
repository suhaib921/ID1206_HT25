#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char** argv){

  // --- Argument Parsing ---
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <num_pages> [option]\n", argv[0]);
    fprintf(stderr, "  num_pages: Number of pages to allocate (e.g., 4096, 8192, 16384)\n");
    fprintf(stderr, "  option: 1 for normal pages (default), 2 for huge pages\n");
    return 1;
  }

  clock_t start, end;
    
  int num_pages = atoi(argv[1]);
  int option = (argc >= 3) ? atoi(argv[2]) : 1;
  // Get the system's standard page size (usually 4KB).
  int page_size = getpagesize();

  printf("Allocating %d pages of %d bytes\n", num_pages, page_size);
  printf("Using Option %d: %s\n", option, (option == 2) ? "Huge pages" : "Normal pages");

  char *addr;

  start = clock();

  // --- Memory Allocation ---
  if (option == 2) {
    // Option 2: Attempt to allocate using huge pages.
    // MAP_HUGETLB flag requests pages of a much larger size (e.g., 2MB or 1GB).
    // This can improve performance by reducing TLB (Translation Lookaside Buffer) misses.
    addr = (char*) mmap(NULL,
                        num_pages * (size_t)page_size,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                        -1,
                        0);
    // Fallback mechanism if huge page allocation fails.
    if (addr == MAP_FAILED) {
      fprintf(stderr, "Error: Huge pages allocation failed.\n");
      fprintf(stderr, "This typically means no huge pages are configured on your system.\n");
      fprintf(stderr, "Check with: cat /proc/meminfo | grep HugePages_Total\n");
      fprintf(stderr, "To configure: sudo sysctl vm.nr_hugepages=<number>\n");
      fprintf(stderr, "Falling back to normal pages...\n\n");
      
      // Retry with normal pages.
      option = 1; 
      addr = (char*) mmap(NULL,
                          num_pages * (size_t)page_size,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS,
                          -1,
                          0);
    }
  } else {
    // Option 1: Allocate using standard system pages (typically 4KB).
    // MAP_PRIVATE: Creates a private copy-on-write mapping.
    // MAP_ANONYMOUS: The mapping is not backed by any file.
    addr = (char*) mmap(NULL,
                        num_pages * (size_t)page_size,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1,
                        0);
  }

  // Check if the final mmap call was successful.
  if (addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  printf("Successfully allocated memory using %s\n",
         (option == 2) ? "huge pages" : "normal pages");

  // --- Triggering Page Faults (Demand Paging) ---
  // The loop below writes one byte to the beginning of each page.
  // mmap only reserves virtual address space. Physical memory is not allocated
  // until the page is first accessed (written to). This first access triggers a
  // 'minor page fault', prompting the kernel to map a physical page.
  char c = 'a';
  for(int i=0; i<num_pages; i++){
    addr[i*(size_t)page_size] = c;
    c ++;
    if (c > 'z') c = 'a'; // cycle characters
  }

  end = clock();

  // Measure CPU time used by the process, not wall-clock time.
  double elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
  double elapsed_cycles = (double)(end - start);

  printf("Elapsed time: %.6f seconds\n", elapsed_time);
  printf("Elapsed cycles: %.0f\n", elapsed_cycles);
  
  // Verify that the writes were successful.
  for(int i=0; (i<num_pages && i<16); i++){
    printf("%c ", addr[i*(size_t)page_size]);
  }
  printf("\n");

  // --- Cleanup ---
  munmap(addr, (size_t)page_size * num_pages);

  return 0;
}
