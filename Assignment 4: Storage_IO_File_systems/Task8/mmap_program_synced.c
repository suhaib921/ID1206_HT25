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
        printf("Child process: mmap returned address = %p\n", (void*)mmaped_ptr);

        // Child writes '01234' to mmaped_ptr[0]
        memcpy(mmaped_ptr, "01234", 5);
        printf("Child: Wrote '01234' to mmaped_ptr[0]\n");

        // Force synchronization to disk using msync()
        if (msync(mmaped_ptr, FILE_SIZE, MS_SYNC) == -1) {
            perror("Child: msync failed");
        } else {
            printf("Child: msync() completed - data flushed to disk\n");
        }

        // Delay to ensure parent writes after child's msync
        sleep(1);

        // Child reads 5 characters from mmaped_ptr[4096]
        char buffer[6] = {0};
        memcpy(buffer, mmaped_ptr + 4096, 5);
        printf("Child: Read '%s' from mmaped_ptr[4096]\n", buffer);

        munmap(mmaped_ptr, FILE_SIZE);
        exit(0);
    }
    else {
        // Parent process
        printf("Parent process: mmap returned address = %p\n", (void*)mmaped_ptr);

        // Wait a bit to ensure child writes first
        sleep(1);

        // Parent writes '56789' to mmaped_ptr[4096]
        memcpy(mmaped_ptr + 4096, "56789", 5);
        printf("Parent: Wrote '56789' to mmaped_ptr[4096]\n");

        // Force synchronization to disk using msync()
        if (msync(mmaped_ptr + 4096, 5, MS_SYNC) == -1) {
            perror("Parent: msync failed");
        } else {
            printf("Parent: msync() completed - data flushed to disk\n");
        }

        // Parent reads 5 characters from mmaped_ptr[0]
        char buffer[6] = {0};
        memcpy(buffer, mmaped_ptr, 5);
        printf("Parent: Read '%s' from mmaped_ptr[0]\n", buffer);

        // Wait for child to finish
        wait(NULL);

        munmap(mmaped_ptr, FILE_SIZE);
    }

    close(fd);
    return 0;
}
