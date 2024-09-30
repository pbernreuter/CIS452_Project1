#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sigint_handler(int sigNum);

int main() {

    signal(SIGINT, sigint_handler);

//~~~~~~~~~~~~~~~~~~~~~~~~ Number of Nodes from User~~~~~~~~~~~~~~~~~~~~~~~~
    int numberOfNodes;
    printf("How many nodes do you want to create?\n");
    if (scanf("%d", &numberOfNodes) != 1 || numberOfNodes < 1) {
        printf("Invalid input\n");
        return 1;
    }

//~~~~~~~~~~~~~~~~~~~~~~~~ Pipes for Parent ~~~~~~~~~~~~~~~~~~~~~~~~
    int parentToFirstChild[2]; // Pipe for Parent to Child 0
    int lastChildToParent[2];  // Pipe for Last Child to Parent
    if (pipe(parentToFirstChild) == -1 || pipe(lastChildToParent) == -1) {
        perror("pipe");
        exit(1);
    }

    int previousPipe[2];

//~~~~~~~~~~~~~~~~~~~~~~~~ Node Creation ~~~~~~~~~~~~~~~~~~~~~~~~
    for (int i = 0; i < numberOfNodes; i++) {

        int nextPipe[2];
        if (i < numberOfNodes - 1 && pipe(nextPipe) == -1) {
            perror("pipe");
            exit(1);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            
            char message[500];

            if ( i == 0){ //First child
                read(parentToFirstChild[0], message, sizeof(message));
                printf("Child %d received message: %s\n", i, message);
                write(nextPipe[1], message, sizeof(message));
            } else if (i == numberOfNodes - 1) { //Last child
                read(previousPipe[0], message, sizeof(message));
                printf("Child %d received message: %s\n", i, message);
                write(lastChildToParent[1], message, sizeof(message));
            } else { //Middle children
                read(previousPipe[0], message, sizeof(message));
                printf("Child %d received message: %s\n", i, message);
                write(nextPipe[1], message, sizeof(message));

            }
        } else {
            // Parent process

            if (i > 0) {
                // After forking child i, close the pipe used by the previous child
                close(previousPipe[0]); // Close read end for the previous child
                close(previousPipe[1]); // Close write end for the previous child
            }

            // Update previousPipe to the newly created nextPipe for the next iteration
            previousPipe[0] = nextPipe[0];
            previousPipe[1] = nextPipe[1];
        }
    }
    while (1) {
        char message[100];

        // Parent sends a message to the first child
        printf("Enter a message to send to the ring: ");
        scanf(" %[^\n]s", message);  // Read a string message from the user

        write(parentToFirstChild[1], message, sizeof(message));
        printf("Parent (Node 0) sent message to child 0\n");

        // Parent waits to receive the message back from the last child
        read(lastChildToParent[0], message, sizeof(message));
        printf("Parent received message back: %s\n", message);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~ Signal Handling ~~~~~~~~~~~~~~~~~~~~~~~~
    void sigint_handler(int sigNum) {
    printf("Signal Recieved. Exiting...\n");
    exit(0);
}
