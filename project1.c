#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sigint_handler(int sigNum);

int main() {
    signal(SIGINT, sigint_handler);

    //~~~~~~~~~~~~~~~~~~~~~~~~ Number of Nodes from User ~~~~~~~~~~~~~~~~~~~~~~~~
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
            // Child process
            char message[500];

            if (i == 0) { // First child
                read(parentToFirstChild[0], message, sizeof(message));
                printf("Child %d received message: %s\n", i, message);
                write(nextPipe[1], message, sizeof(message)); // Pass message to next child
                printf("Child %d sent message: %s\n", i, message);
            } else if (i == numberOfNodes - 1) { // Last child
                read(previousPipe[0], message, sizeof(message));
                printf("Child %d received message: %s\n", i, message);
                write(lastChildToParent[1], message, sizeof(message)); // Send message back to parent
                printf("Child %d sent message: %s\n", i, message);
            } else { // Middle children
                read(previousPipe[0], message, sizeof(message));
                printf("Child %d received message: %s\n", i, message);
                write(nextPipe[1], message, sizeof(message)); // Pass message to next child
                printf("Child %d sent message: %s\n", i, message);
            }
            //exit(0); // Exit child process after handling
        } else {
            // Parent process
            if (i > 0) {
                // After forking child i, close the pipe used by the previous child
                close(previousPipe[0]); // Close read end for the previous child
                close(previousPipe[1]); // Close write end for the previous child
            }

            // Update previousPipe to the newly created nextPipe for the next iteration
            if (i < numberOfNodes - 1) {
                previousPipe[0] = nextPipe[0];
                previousPipe[1] = nextPipe[1];
            }
        }
    }

    while (1) {
        char message[100];
        int targetNode;

        // Parent sends a message to the specified child
        printf("Enter a message to send to the ring: ");
        scanf(" %[^\n]s", message);  // Read a string message from the user

        printf("Enter the target node (0 to %d): ", numberOfNodes - 1);
        scanf("%d", &targetNode);

        // Validate target node
        if (targetNode < 0 || targetNode >= numberOfNodes) {
            printf("Invalid target node. Please enter a number between 0 and %d.\n", numberOfNodes - 1);
            continue; // Skip the rest of the loop
        }

        // Send message to the first child
        write(parentToFirstChild[1], message, sizeof(message));
        printf("Parent (Node 0) sent message to child 0\n");

        // Parent waits to receive the message back from the last child
        char response[500];
        int currentNode = 0;

        // Process the messages from children until it reaches the target node
        while (1) {
            read(lastChildToParent[0], response, sizeof(response));
            printf("Parent received message: %s from Child %d\n", response, currentNode);
            
            // Check if the current node is the target node
            if (currentNode == targetNode) {
                printf("Parent received final message back from target Child %d: %s\n", targetNode, response);
                break; // Exit the loop after receiving the target node's response
            }

            // Move to the next child in the ring
            currentNode = (currentNode + 1) % numberOfNodes;
        }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~ Signal Handling ~~~~~~~~~~~~~~~~~~~~~~~~ 
void sigint_handler(int sigNum) {
    printf("Signal Received. Exiting...\n");
    exit(0);
}
