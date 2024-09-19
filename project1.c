#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sigint_handler(int sigNum);

int main() {

//~~~~~~~~~~~~~~~~~~~~~~~~ Number of Nodes from User~~~~~~~~~~~~~~~~~~~~~~~~
    int numberOfNodes;
    printf("How many nodes do you want to create?\n");
    if (scanf("%d", &numberOfNodes) != 1 || numberOfNodes < 1) {
        printf("Invalid input\n");
        return 1;
    }

//~~~~~~~~~~~~~~~~~~~~~~~~ Pipe Creation ~~~~~~~~~~~~~~~~~~~~~~~~
    int pipes[numberOfNodes+1][2];
    for (int i = 0; i < numberOfNodes+1; i++) {
        if (pipe(pipes[i]) < 0) {
            printf("Pipe creation failed\n");
            return 1;
        }
    }

//~~~~~~~~~~~~~~~~~~~~~~~~ Node Creation ~~~~~~~~~~~~~~~~~~~~~~~~
    for (int i = 0; i < numberOfNodes; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            printf("Child %d created\n", i);
            break;
        }
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~ Signal Handling ~~~~~~~~~~~~~~~~~~~~~~~~
    void sigint_handler(int sigNum) {
    printf("Signal Recieved. Exiting...\n");
    exit(0);
}
