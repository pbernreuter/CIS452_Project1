#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAX_MSG_LEN 256

//struct for msg
typedef struct {
    char message[MAX_MSG_LEN];
    int targetNode;
    int senderNode;
} Message;

int k;  //# of nodes
int **pipes; 
int parentPID;

//signal handler for graceful shutdown
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

int main() {
    printf("Enter the number of nodes (including parent): ");
    scanf("%d", &k);
    getchar();  //get rid newline character left

    //setup signal handling
    parentPID = getpid();
    signal(SIGINT, signalHandler);
    pipes = malloc(k * sizeof(int*));
    for (int i = 0; i < k; ++i) {
        pipes[i] = malloc(2 * sizeof(int));
        pipe(pipes[i]); 
    }

    //create nodes
    for (int i = 1; i < k; ++i) {
        if (fork() == 0) {
            //child process
            int node = i;
            Message msg;
            while (1) {
                //read prev. node
                read(pipes[node][0], &msg, sizeof(Message));

                if (msg.targetNode != -1) {
                    printf("Node %d received message from Node %d: \"%s\" (Target: Node %d)\n", 
                            node, node - 1, msg.message, msg.targetNode);

                    if (msg.targetNode == node) {
                        printf("Node %d: Message received and processed.\n", node);
                        //clear the message after processing
                        msg.targetNode = -1;
                        strcpy(msg.message, "empty");
                    }
                }
                //forward to next node
                write(pipes[(node + 1) % k][1], &msg, sizeof(Message));
            }
        }
    }
    //parent process -- message input and passing
    Message msg;
    char buffer[MAX_MSG_LEN];

    while (1) {
        //clear buffer before new input
        memset(buffer, 0, MAX_MSG_LEN);

        printf("Enter the message: ");
        fgets(buffer, MAX_MSG_LEN, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  //remove newline
        printf("Enter target node (0 to %d): ", k-1);
        int dest;
        scanf("%d", &dest);
        getchar();  //clear newline leftover
        strcpy(msg.message, buffer);
        msg.targetNode = dest;
        msg.senderNode = 0;

        //send first msg
        write(pipes[1][1], &msg, sizeof(Message));

        //wait for priority to return to parent
        read(pipes[0][0], &msg, sizeof(Message));
        printf("Priority returned to parent. Ready for next message.\n");
    }


    return 0;
}
