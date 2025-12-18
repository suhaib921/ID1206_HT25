#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>

#define FILE_SIZE (1024 * 1024)  // 1 MB
#define FILE_NAME "file_to_map.txt"

int main() {
    int fd;
    char *mmaped_ptr;
    pid_t pid;
    sem_t *sem_child_done, *sem_parent_done;

    // Create named semaphores for synchronization
    sem_unlink("/sem_child_done");
    sem_unlink("/sem_parent_done");

    sem_child_done = sem_open("/sem_child_done", O_CREAT | O_EXCL, 0644, 0);
    sem_parent_done = sem_open("/sem_parent_done", O_CREAT | O_EXCL, 0644, 0);

    if (sem_child_done == SEM_FAILED || sem_parent_done == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

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

        // Signal that child is done writing
        sem_post(sem_child_done);
        printf("Child: Signaled write completion\n");

        // Wait for parent to finish writing
        sem_wait(sem_parent_done);
        printf("Child: Parent write confirmed\n");

        // Child reads 5 characters from mmaped_ptr[4096]
        char buffer[6] = {0};
        memcpy(buffer, mmaped_ptr + 4096, 5);
        printf("Child: Read '%s' from mmaped_ptr[4096]\n", buffer);

        munmap(mmaped_ptr, FILE_SIZE);
        sem_close(sem_child_done);
        sem_close(sem_parent_done);
        exit(0);
    }
    else {
        // Parent process
        printf("Parent process: mmap returned address = %p\n", (void*)mmaped_ptr);

        // Wait for child to finish writing before parent writes
        sem_wait(sem_child_done);
        printf("Parent: Child write confirmed\n");

        // Parent writes '56789' to mmaped_ptr[4096]
        memcpy(mmaped_ptr + 4096, "56789", 5);
        printf("Parent: Wrote '56789' to mmaped_ptr[4096]\n");

        // Signal that parent is done writing
        sem_post(sem_parent_done);
        printf("Parent: Signaled write completion\n");

        // Parent reads 5 characters from mmaped_ptr[0]
        char buffer[6] = {0};
        memcpy(buffer, mmaped_ptr, 5);
        printf("Parent: Read '%s' from mmaped_ptr[0]\n", buffer);

        // Wait for child to finish
        wait(NULL);

        munmap(mmaped_ptr, FILE_SIZE);
        sem_close(sem_child_done);
        sem_close(sem_parent_done);
        sem_unlink("/sem_child_done");
        sem_unlink("/sem_parent_done");
    }

    close(fd);
    return 0;
}
