#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define FILE_SIZE (1024 * 1024)  // 1 MB
#define FILE_NAME "file_to_map.txt"

int main() {
    int fd;
    char *mmaped_ptr;
    pid_t pid;

    // Open the file
    fd = open(FILE_NAME, O_RDWR);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // Create memory mapping with shared flag
    mmaped_ptr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmaped_ptr == MAP_FAILED) {
        perror("Error mmapping the file");
        close(fd);
        exit(1);
    }

    // Fork to create child process
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        munmap(mmaped_ptr, FILE_SIZE);
        close(fd);
        exit(1);
    }
    else if (pid == 0) {
        // Child process
        printf("Child process (pid=%d); mmap address: %p\n", getpid(), (void*)mmaped_ptr);

        // Child writes '01234' to mmaped_ptr[0]
        memcpy(mmaped_ptr, "01234", 5);

        // Small delay to allow some interleaving
        usleep(100);

        // Child reads 5 characters from mmaped_ptr[4096]
        char buffer[6] = {0};
        memcpy(buffer, mmaped_ptr + 4096, 5);
        printf("Child process (pid=%d); read from mmaped_ptr[4096]: %s\n", getpid(), buffer);

        munmap(mmaped_ptr, FILE_SIZE);
        exit(0);
    }
    else {
        // Parent process
        printf("Parent process (pid=%d); mmap address: %p\n", getpid(), (void*)mmaped_ptr);

        // Parent writes '56789' to mmaped_ptr[4096]
        memcpy(mmaped_ptr + 4096, "56789", 5);

        // Small delay to allow some interleaving
        usleep(100);

        // Parent reads 5 characters from mmaped_ptr[0]
        char buffer[6] = {0};
        memcpy(buffer, mmaped_ptr, 5);
        printf("Parent process (pid=%d); read from mmaped_ptr[0]: %s\n", getpid(), buffer);

        // Wait for child to finish
        wait(NULL);

        munmap(mmaped_ptr, FILE_SIZE);
    }

    close(fd);
    return 0;
}
