#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define NUM_BINS 30
#define RANGE_MIN 0.0
#define RANGE_MAX 1.0

int num_threads = 0;
int array_length = 0;
double *array;
int *global_histogram;
int **local_histograms;

typedef struct {
    int thread_id;
    int start_idx;
    int end_idx;
} thread_data_t;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void initialize_array() {
    array = (double *)malloc(array_length * sizeof(double));
    if (array == NULL) {
        perror("Failed to allocate array");
        exit(1);
    }
    
    srand(time(NULL));
    for (int i = 0; i < array_length; i++) {
        array[i] = RANGE_MIN + ((double)rand() / RAND_MAX) * (RANGE_MAX - RANGE_MIN);
    }
}

void print_histogram(int *histogram, const char *title) {
    printf("\n%s:\n", title);
    printf("Bin\tRange\t\tCount\n");
    printf("---\t-------------\t-----\n");
    
    double bin_width = (RANGE_MAX - RANGE_MIN) / NUM_BINS;
    for (int i = 0; i < NUM_BINS; i++) {
        double lower = RANGE_MIN + i * bin_width;
        double upper = lower + bin_width;
        printf("%2d\t[%6.4f, %6.4f)\t%5d\n", i, lower, upper, histogram[i]);
    }
    
    // Verify total count
    int total = 0;
    for (int i = 0; i < NUM_BINS; i++) {
        total += histogram[i];
    }
    printf("Total elements: %d\n", total);
}

void serial_histogram() {
    double start_time = get_time();
    
    // Initialize histogram
    int *histogram = (int *)calloc(NUM_BINS, sizeof(int));
    if (histogram == NULL) {
        perror("Failed to allocate histogram");
        exit(1);
    }
    
    double bin_width = (RANGE_MAX - RANGE_MIN) / NUM_BINS;
    
    // Build histogram
    for (int i = 0; i < array_length; i++) {
        int bin = (int)((array[i] - RANGE_MIN) / bin_width);
        if (bin >= NUM_BINS) bin = NUM_BINS - 1; // Handle edge case
        histogram[bin]++;
    }
    
    double end_time = get_time();
    double elapsed = end_time - start_time;
    
    print_histogram(histogram, "Serial Histogram");
    printf("Serial time: %.6f seconds\n", elapsed);
    
    free(histogram);
}

void *thread_histogram(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int my_id = data->thread_id;
    
    double bin_width = (RANGE_MAX - RANGE_MIN) / NUM_BINS;
    
    // Initialize local histogram for this thread
    for (int i = 0; i < NUM_BINS; i++) {
        local_histograms[my_id][i] = 0;
    }
    
    // Build local histogram for assigned portion
    for (int i = data->start_idx; i < data->end_idx; i++) {
        int bin = (int)((array[i] - RANGE_MIN) / bin_width);
        if (bin >= NUM_BINS) bin = NUM_BINS - 1;
        local_histograms[my_id][bin]++;
    }
    
    pthread_exit(NULL);
}

void parallel_histogram() {
    double start_time = get_time();
    
    // Initialize global histogram
    for (int i = 0; i < NUM_BINS; i++) {
        global_histogram[i] = 0;
    }
    
    // Allocate thread data and create threads
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_data_t *thread_data = (thread_data_t *)malloc(num_threads * sizeof(thread_data_t));
    
    if (threads == NULL || thread_data == NULL) {
        perror("Failed to allocate thread resources");
        exit(1);
    }
    
    // Calculate work distribution
    int chunk_size = array_length / num_threads;
    int remainder = array_length % num_threads;
    int current_start = 0;
    
    // Phase 1: Parallel Computation (No Synchronization)
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].start_idx = current_start;
        thread_data[i].end_idx = current_start + chunk_size + (i < remainder ? 1 : 0);
        current_start = thread_data[i].end_idx;
        
        pthread_create(&threads[i], NULL, thread_histogram, (void *)&thread_data[i]);
    }
    
    // Phase 2: Barrier Synchronization  
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL); 
    }
    
    // Phase 3: Serial Reduction (No Synchronization)
    // Aggregate local histograms into global histogram
    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < NUM_BINS; j++) {
            global_histogram[j] += local_histograms[i][j];
        }
    }
    
    double end_time = get_time();
    double elapsed = end_time - start_time;
    
    print_histogram(global_histogram, "Parallel Histogram");
    printf("Parallel time: %.6f seconds\n", elapsed);
    
    free(threads);
    free(thread_data);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <num_threads> <array_length>\n", argv[0]);
        exit(1);
    }

    double total_start = get_time();   

    
    num_threads = atoi(argv[1]);
    array_length = atoi(argv[2]);
    
    printf("Running with %d threads, array length: %d\n", num_threads, array_length);
    
    // Initialize array with random values
    initialize_array();
    
    // Allocate memory for histograms
    global_histogram = (int *)calloc(NUM_BINS, sizeof(int));
    local_histograms = (int **)malloc(num_threads * sizeof(int *));
    
    if (global_histogram == NULL || local_histograms == NULL) {
        perror("Failed to allocate histogram memory");
        exit(1);
    }
    
    for (int i = 0; i < num_threads; i++) {
        local_histograms[i] = (int *)calloc(NUM_BINS, sizeof(int));
        if (local_histograms[i] == NULL) {
            perror("Failed to allocate local histogram");
            exit(1);
        }
    }
    
    // Run serial histogram
    serial_histogram();
    
    // Run parallel histogram
    parallel_histogram();
    
    free(array);
    free(global_histogram);
    for (int i = 0; i < num_threads; i++) {
        free(local_histograms[i]);
    }
    free(local_histograms);

    double total_end = get_time();   // End overall timer
    printf("Total program time: %.6f seconds\n", total_end - total_start);

    
    return 0;
}