#include <stdio.h>
#include <unistd.h>

#define N 3

int main() {
    for (int i = 0; i < N; i++) {
        fork();
        fork();
    }

    printf("PID: %d, PPID: %d\n", getpid(), getppid());
    sleep(5); // gives time to see processes
    return 0;
}
