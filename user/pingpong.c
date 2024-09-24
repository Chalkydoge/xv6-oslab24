#include "kernel/types.h"
#include "user/user.h"
#define BUFSIZE 32

int main() {
    int p1[2]; // Pipe 1: parent to child (ping)
    int p2[2]; // Pipe 2: child to parent (pong)

    pipe(p1);  // Create first pipe
    pipe(p2);  // Create second pipe

    int pid = fork();
    if (pid < 0) {
        // Fork failed
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // Child process
        char buf[BUFSIZE];
        
        // Close unused ends of the pipes
        close(p1[1]);  // Close write end of p1
        close(p2[0]);  // Close read end of p2

        // Read "ping" from parent
        if (read(p1[0], buf, BUFSIZE) > 0) {
            int parent_pid = atoi(buf);  // Convert parent PID from string to integer
            printf("%d: received ping from pid %d\n", getpid(), parent_pid);
        }
        
        // Send "pong" to parent
        itoa(getpid(), buf);  // Convert child PID to string
        write(p2[1], buf, strlen(buf) + 1);  // Write PID as string

        // Close used pipe ends
        close(p1[0]);  // Close read end of p1
        close(p2[1]);  // Close write end of p2

        exit(0);
    } else {
        // Parent process
        char buf[BUFSIZE];
        
        // Close unused ends of the pipes
        close(p1[0]);  // Close read end of p1
        close(p2[1]);  // Close write end of p2

        // Send "ping" to child
        itoa(getpid(), buf);  // Convert parent PID to string
        write(p1[1], buf, strlen(buf) + 1);  // Write PID as string
        
        // Wait for "pong" from child
        if (read(p2[0], buf, BUFSIZE) > 0) {
            int child_pid = atoi(buf);  // Convert child PID from string to integer
            printf("%d: received pong from pid %d\n", getpid(), child_pid);
        }

        // Close used pipe ends
        close(p1[1]);  // Close write end of p1
        close(p2[0]);  // Close read end of p2

        // Wait for the child process to finish
        wait(0);

        exit(0);
    }
}