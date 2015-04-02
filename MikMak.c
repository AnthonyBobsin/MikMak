#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Request struct
struct request {
    long mtype;
    char mtext[103];
};

// MikMak struct
struct mikMak {
    long mtype;
    char text[100];
    int points;
};

// All MikMaks
struct allMikMaks {
    long mtype;
    // Array of all the Mik Maks
    struct mikMak maks[15];
    int count;
};

int main(void)
{
    struct request req;
    struct allMikMaks allMaks;
    int msqid, i;
    key_t key;

    if ((key = ftok("MikMak.c", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    req.mtype = 1; /* We don't really care for request */
    // Make refresh request at start
    strcpy(req.mtext, "R");
    if (msgsnd(msqid, &req, sizeof(req.mtext), 0) == -1) {
        perror("msgsnd");
    }
    printf("Start the server to get started\n");
    // Refresh at start
    if (req.mtext[0] == 'R') {
        // Wait for message response
        for (;;) {
            if (msgrcv(msqid, &allMaks, sizeof(allMaks.maks) + sizeof(int), 2, 0) == -1) {
                perror("msgrcv");
                exit(1);
            }
            system("clear");
            printf("----------------------------------------------------------------------------------\n");
            for (i = 0; i < allMaks.count; i++) {
            // Print positive version
            if (allMaks.maks[i].points >= 0)
                printf("%d. +%d %s\n", i + 1, allMaks.maks[i].points, allMaks.maks[i].text);
            // Print negative version
            else
                printf("%d. %d %s\n", i + 1, allMaks.maks[i].points, allMaks.maks[i].text);
            }
            break;
        }
    }

    for (;;) {
        printf("[R]efresh [U]pvote [D]ownvote [S]end [E]xit:");
        // Wait for request
        if (fgets(req.mtext, sizeof(req.mtext), stdin) != NULL) {
            int len = strlen(req.mtext);

            // ditch newline at end, if it exists 
            if (req.mtext[len-1] == '\n') req.mtext[len-1] = '\0';

            // Request is valid && not exit
            if (req.mtext[0] == 'R' || req.mtext[0] == 'U' || req.mtext[0] == 'D'
                || req.mtext[0] == 'S') {
                // Send message with request
                if (msgsnd(msqid, &req, sizeof(req.mtext), 0) == -1) {
                    perror("msgsnd");
                }
                // If user wants a refresh
                if (req.mtext[0] == 'R') {
                    // Wait for message response
                    for (;;) {
                        if (msgrcv(msqid, &allMaks, sizeof(allMaks.maks) + sizeof(int), 2, 0) == -1) {
                            perror("msgrcv");
                            exit(1);
                        }
                        system("clear");
                        printf("----------------------------------------------------------------------------------\n");
                        for (i = 0; i < allMaks.count; i++) {
                            // Print positive version
                            if (allMaks.maks[i].points >= 0)
                                printf("%d. +%d %s\n", i + 1, allMaks.maks[i].points, allMaks.maks[i].text);
                            // Print negative version
                            else
                                printf("%d. %d %s\n", i + 1, allMaks.maks[i].points, allMaks.maks[i].text);
                        }
                        break;
                    }
                }
            }
            // Request is exit
            else if (req.mtext[0] == 'E') {
                // Clean up the message queue
                /*if (msgctl(msqid, IPC_RMID, NULL) == -1) {
                    perror("msgctl");
                    exit(1);
                }*/
                printf("----------------------------------------------------------------------------------\n");
                printf("Goodbye!\n");
                printf("----------------------------------------------------------------------------------\n");
                exit(0);
            }
            // Request is invalid
            else {
                printf("Not a valid input, please try again.\n");
            }
        }
    }

    return 0;
}

