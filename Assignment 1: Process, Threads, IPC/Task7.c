#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 3

int main() {
    
    int pid = fork();
    if(pid == 0) {
        printf("I'm the child %d\n", getpid());
    } else {
        printf("My child is called %d\n", pid);
        wait(NULL); // Wait for the child to exit
        sleep(10);
    }
    printf("PID%d says goodbye\n", getpid());

    return 0;
}