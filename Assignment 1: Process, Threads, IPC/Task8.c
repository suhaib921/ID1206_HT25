#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

// Function to generate a random number between 0 and 1
double rand_in_range() {
    return (double)rand() / RAND_MAX;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        fprintf(stderr, "Where N is the size of the array\n");
        return 1;
    }

    // Convert argument to integer
    int N = atoi(argv[1]);
    if (N <= 0) {
        fprintf(stderr, "N must be positive.\n");
        return 1;
    }

    // Allocate memory for an array of N doubles
    double *array = malloc(N * sizeof(double));
    if (!array) {
        perror("malloc failed");
        return 1;
    }

    // Seed random number generator and fill array with random values
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        array[i] = rand_in_range();
    }

    // Two pipes for inter-process communication
    // pipe1 for first child, pipe2 for second child
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe failed");
        free(array);
        return 1;
    }

    // Start timer to measure total execution time
    struct timeval start, end;
    gettimeofday(&start, NULL);

    printf("Parent process PID: %d\n", getpid());
    printf("Creating 2 child processes...\n");

    // ======== CHILD 1 CREATION ========
    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("fork failed");
        free(array);
        return 1;
    }

    if (pid1 == 0) {
        // ---------- CHILD 1 ----------
        // Summing the first half of the array

        struct timeval cstart, cend;
        gettimeofday(&cstart, NULL); 

        close(pipe1[0]);  // Close unused read end of pipe

        double sum1 = 0;
        for (int i = 0; i < N / 2; i++) {
            sum1 += array[i];
        }

        // Record child end time
        gettimeofday(&cend, NULL);
        double child_time = (cend.tv_sec - cstart.tv_sec) * 1000.0;
        child_time += (cend.tv_usec - cstart.tv_usec) / 1000.0;

        printf("[Child 1 - PID %d] Sum = %.6f, Time = %.3f ms\n", getpid(), sum1, child_time);

        // Send the result back to the parent through the pipe
        if (write(pipe1[1], &sum1, sizeof(double)) < 0)
            perror("write error in child 1");

        close(pipe1[1]);
        free(array);
        exit(0);
    }


    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork failed");
        free(array);
        return 1;
    }

    if (pid2 == 0) {
        // ---------- CHILD 2 ----------
        // Summing the second half of the array

        struct timeval cstart, cend;
        gettimeofday(&cstart, NULL); 

        close(pipe2[0]); 

        double sum2 = 0;
        for (int i = N / 2; i < N; i++) {
            sum2 += array[i];
        }

        gettimeofday(&cend, NULL);
        double child_time = (cend.tv_sec - cstart.tv_sec) * 1000.0;
        child_time += (cend.tv_usec - cstart.tv_usec) / 1000.0;

        printf("[Child 2 - PID %d] Sum = %.6f, Time = %.3f ms\n", getpid(), sum2, child_time);

        // Send result back to parent
        if (write(pipe2[1], &sum2, sizeof(double)) < 0)
            perror("write error in child 2");

        // Cleanup before exit
        close(pipe2[1]);
        free(array);
        exit(0);
    }


    // ======== PARENT PROCESS ========
    // The parent now reads both results and combines them

    close(pipe1[1]);  // Close write end of pipe1
    close(pipe2[1]);  // Close write end of pipe2

    double sum1, sum2;

    // Read results from both children
    if (read(pipe1[0], &sum1, sizeof(double)) < 0)
        perror("read error pipe1");
    if (read(pipe2[0], &sum2, sizeof(double)) < 0)
        perror("read error pipe2");

    close(pipe1[0]);
    close(pipe2[0]);

    // Wait for both children to finish execution
    wait(NULL);
    wait(NULL);

    // Combine results from both halves
    double total_sum = sum1 + sum2;

    // Stop total timer
    gettimeofday(&end, NULL);

    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsed += (end.tv_usec - start.tv_usec) / 1000.0;

    printf("\n--- Summary ---\n");
    printf("Number of child processes: 2\n");
    printf("Total sum of array (N=%d): %.6f\n", N, total_sum);
    printf("Total execution time: %.3f ms\n\n", elapsed);

    // Free memory and exit
    free(array);
    return 0;
}
