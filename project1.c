#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAX_MSG_LEN 256

// Structure for message
typedef struct {
    char message[MAX_MSG_LEN];
    int targetNode;
    int senderNode;
} Message;

int k;  // Number of nodes (including parent)
int **pipes;  // Pipes for communication
int parentPID;  // Parent process ID

// Signal handler for graceful shutdown
void signalHandler(int sig) {
    if (getpid() == parentPID) {
        printf("Shutting down simulation...\n");
        for (int i = 0; i < k; ++i) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        exit(0);
    }
}

// Function to create child processes and setup the ring
void createProcesses() {
    for (int i = 1; i < k; ++i) {
        if (fork() == 0) {
            // Child process
            int node = i;
            Message msg;
            while (1) {
                // Read from previous node
                read(pipes[node][0], &msg, sizeof(Message));

                if (msg.targetNode != -1) {
                    printf("Node %d received message from Node %d: \"%s\" (Target: Node %d)\n", 
                            node, msg.senderNode, msg.message, msg.targetNode);

                    // Check if the message is for this node
                    if (msg.targetNode == node) {
                        printf("Node %d: Message received and processed.\n", node);
                        // Clear the message after processing
                        msg.targetNode = -1;
                        strcpy(msg.message, "empty");
                    }
                }

                // Forward the message to the next node
                write(pipes[(node + 1) % k][1], &msg, sizeof(Message));
            }
        }
    }
}

// Parent process for sending messages
void parentProcess() {
    Message msg;
    char buffer[MAX_MSG_LEN];

    while (1) {
        // Clear buffer before taking new input
        memset(buffer, 0, MAX_MSG_LEN);

        // Get message from user
        printf("Enter the message: ");
        fgets(buffer, MAX_MSG_LEN, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove newline character

        // Get target node from user
        printf("Enter the destination node (0 to %d): ", k-1);
        int dest;
        scanf("%d", &dest);
        getchar();  // Consume the newline character left by scanf

        // Prepare the message
        strcpy(msg.message, buffer);
        msg.targetNode = dest;
        msg.senderNode = 0;

        // Send the message to the first node (Node 1)
        write(pipes[1][1], &msg, sizeof(Message));

        // Wait for the apple to return to parent
        read(pipes[0][0], &msg, sizeof(Message));
        printf("Parent received the apple back. Ready for next message.\n");
    }
}

int main() {
    printf("Enter the number of nodes (including parent): ");
    scanf("%d", &k);
    getchar();  // Consume the newline character left by scanf

    // Setup signal handling
    parentPID = getpid();
    signal(SIGINT, signalHandler);

    // Create pipes for communication
    pipes = malloc(k * sizeof(int*));
    for (int i = 0; i < k; ++i) {
        pipes[i] = malloc(2 * sizeof(int));
        pipe(pipes[i]);  // Create pipe
    }

    // Create child processes (nodes)
    createProcesses();

    // Parent process controls message input and passing
    parentProcess();

    return 0;
}
