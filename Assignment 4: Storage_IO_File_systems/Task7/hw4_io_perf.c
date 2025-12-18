#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

typedef struct {
    long offset;
    int bytes;
} Request;

typedef struct {
    int fd;
    Request *requests;
    int num_requests;
    char *buffer;
} ThreadArgs;

void *reader_thread_func(void *arg) {
     ThreadArgs *args = (ThreadArgs *)arg;
     char *local_buffer = malloc(16384); // Max chunk size

     for (int i = 0; i < args->num_requests; i++) {
         pread(args->fd, local_buffer, args->requests[i].bytes, args->requests[i].offset);
     }

     free(local_buffer);
     pthread_exit(0);
}

void *writer_thread_func(void *arg) {
     ThreadArgs *args = (ThreadArgs *)arg;

     for (int i = 0; i < args->num_requests; i++) {
         pwrite(args->fd, args->buffer, args->requests[i].bytes, args->requests[i].offset);
     }

     pthread_exit(0);
}

double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

int main(int argc, char *argv[])
{
     if (argc != 3) {
         fprintf(stderr, "Usage: %s <num_bytes> <num_threads>\n", argv[0]);
         return 1;
     }

     long n = atol(argv[1]); // number of bytes
     int p = atoi(argv[2]); // number of threads

     // Create a file for saving the data
     int fd = open("testfile.dat", O_CREAT | O_RDWR | O_TRUNC, 0644);
     if (fd < 0) {
         perror("open");
         return 1;
     }

     // Allocate a buffer and initialize it
     char *buffer = malloc(16384);
     memset(buffer, 'A', 16384);

     // Calculate number of requests for List 1 (sequential 16KB chunks)
     int num_requests_list1 = n / 16384;

     // Create List 1: sequential requests of 16384 bytes
     Request *list1 = malloc(num_requests_list1 * sizeof(Request));
     for (int i = 0; i < num_requests_list1; i++) {
         list1[i].offset = i * 16384;
         list1[i].bytes = 16384;
     }

     // Calculate number of requests for List 2 (random 128 byte chunks)
     int num_requests_list2 = n / 128;

     // Create List 2: random requests of 128 bytes at 4096-aligned offsets
     Request *list2 = malloc(num_requests_list2 * sizeof(Request));
     srand(time(NULL));
     int *used = calloc(num_requests_list2, sizeof(int));

     for (int i = 0; i < num_requests_list2; i++) {
         int idx;
         do {
             idx = rand() % num_requests_list2;
         } while (used[idx]);
         used[idx] = 1;

         list2[i].offset = idx * 4096;
         list2[i].bytes = 128;
     }
     free(used);

     // === TEST WITH LIST 1 (Sequential) ===

     struct timeval start, end;

     // Start timing for writes
     gettimeofday(&start, NULL);

     // Create writer workers and pass in their portion of list1
     pthread_t writers[p];
     ThreadArgs write_args[p];
     int requests_per_thread = num_requests_list1 / p;

     for (int i = 0; i < p; i++) {
         write_args[i].fd = fd;
         write_args[i].requests = &list1[i * requests_per_thread];
         write_args[i].num_requests = (i == p - 1) ? (num_requests_list1 - i * requests_per_thread) : requests_per_thread;
         write_args[i].buffer = buffer;
         pthread_create(&writers[i], NULL, writer_thread_func, &write_args[i]);
     }

     // Wait for all writers to finish
     for (int i = 0; i < p; i++) {
         pthread_join(writers[i], NULL);
     }

     // End timing for writes
     gettimeofday(&end, NULL);
     double write_time = get_time_diff(start, end);
     long write_bytes = num_requests_list1 * 16384L;
     double write_mb = write_bytes / (1024.0 * 1024.0);
     double write_bandwidth = write_mb / write_time;

     // Print out the write bandwidth
     printf("List1: Write %ld bytes, use %d threads, elapsed time %.6f s, write bandwidth: %.2f MB/s\n",
            write_bytes, p, write_time, write_bandwidth);

     // Start timing for reads
     gettimeofday(&start, NULL);

     // Create reader workers and pass in their portion of list1
     pthread_t readers[p];
     ThreadArgs read_args[p];

     for (int i = 0; i < p; i++) {
         read_args[i].fd = fd;
         read_args[i].requests = &list1[i * requests_per_thread];
         read_args[i].num_requests = (i == p - 1) ? (num_requests_list1 - i * requests_per_thread) : requests_per_thread;
         read_args[i].buffer = buffer;
         pthread_create(&readers[i], NULL, reader_thread_func, &read_args[i]);
     }

     // Wait for all readers to finish
     for (int i = 0; i < p; i++) {
         pthread_join(readers[i], NULL);
     }

     // End timing for reads
     gettimeofday(&end, NULL);
     double read_time = get_time_diff(start, end);
     long read_bytes = num_requests_list1 * 16384L;
     double read_mb = read_bytes / (1024.0 * 1024.0);
     double read_bandwidth = read_mb / read_time;

     // Print out the read bandwidth
     printf("List1: Read %ld bytes, use %d threads, elapsed time %.6f s, read bandwidth: %.2f MB/s\n",
            read_bytes, p, read_time, read_bandwidth);

     // === TEST WITH LIST 2 (Random) ===

     // Truncate and reopen file
     close(fd);
     fd = open("testfile.dat", O_CREAT | O_RDWR | O_TRUNC, 0644);

     // Start timing for writes
     gettimeofday(&start, NULL);

     // Create writer workers for list2
     int requests_per_thread_list2 = num_requests_list2 / p;
     for (int i = 0; i < p; i++) {
         write_args[i].fd = fd;
         write_args[i].requests = &list2[i * requests_per_thread_list2];
         write_args[i].num_requests = (i == p - 1) ? (num_requests_list2 - i * requests_per_thread_list2) : requests_per_thread_list2;
         write_args[i].buffer = buffer;
         pthread_create(&writers[i], NULL, writer_thread_func, &write_args[i]);
     }

     for (int i = 0; i < p; i++) {
         pthread_join(writers[i], NULL);
     }

     gettimeofday(&end, NULL);
     write_time = get_time_diff(start, end);
     write_bytes = num_requests_list2 * 128L;
     write_mb = write_bytes / (1024.0 * 1024.0);
     write_bandwidth = write_mb / write_time;

     printf("List2: Write %ld bytes, use %d threads, elapsed time %.6f s, write bandwidth: %.2f MB/s\n",
            write_bytes, p, write_time, write_bandwidth);

     // Start timing for reads
     gettimeofday(&start, NULL);

     for (int i = 0; i < p; i++) {
         read_args[i].fd = fd;
         read_args[i].requests = &list2[i * requests_per_thread_list2];
         read_args[i].num_requests = (i == p - 1) ? (num_requests_list2 - i * requests_per_thread_list2) : requests_per_thread_list2;
         read_args[i].buffer = buffer;
         pthread_create(&readers[i], NULL, reader_thread_func, &read_args[i]);
     }

     for (int i = 0; i < p; i++) {
         pthread_join(readers[i], NULL);
     }

     gettimeofday(&end, NULL);
     read_time = get_time_diff(start, end);
     read_bytes = num_requests_list2 * 128L;
     read_mb = read_bytes / (1024.0 * 1024.0);
     read_bandwidth = read_mb / read_time;

     printf("List2: Read %ld bytes, use %d threads, elapsed time %.6f s, read bandwidth: %.2f MB/s\n",
            read_bytes, p, read_time, read_bandwidth);

     // Free up resources properly
     close(fd);
     free(buffer);
     free(list1);
     free(list2);

     return 0;
}
