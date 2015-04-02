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
    struct allMikMaks allMaks;
    struct request req;
    int msqid, i, voteIndex;
    key_t key;

    if ((key = ftok("MikMak.c", 'B')) == -1) {  /* same key as MikMak.c */
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Fake seed data
    strcpy(allMaks.maks[0].text, "I’ll only need a 413.7\% on my next exam to get an A in the class!");
    allMaks.maks[0].points = 6;
    strcpy(allMaks.maks[1].text, "Why did I do that? A novel by me, with special guest appearances by several adult beverages.");
    allMaks.maks[1].points = -3;
    strcpy(allMaks.maks[2].text, "When you take a 10 minutes study break and it accidentally lasts the entire year.");
    allMaks.maks[2].points = -1;
    strcpy(allMaks.maks[3].text, "Life is too short to eject a USB safely.");
    allMaks.maks[3].points = 14;
    strcpy(allMaks.maks[4].text, "I’m a lot nicer than my \"walking to class\" face. I promise.");
    allMaks.maks[4].points = 9;

    allMaks.count = 5;
    
    printf("Server up and running!\n");

    for(;;) {
        if (msgrcv(msqid, &req, sizeof(req.mtext), 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        // Depending on request, act accordingly
        switch(req.mtext[0]) {
            // User wants a refreshed list of Mik Maks
            case 'R':
                printf("You requested all MikMaks: %s\n", req.mtext);
                // strcpy(req.mtext, "Message Received");
                // req.mtype = 2;
                allMaks.mtype = 2;
                if (msgsnd(msqid, &allMaks, sizeof(allMaks.maks) + sizeof(int), 0) == -1) {
                    perror("msgsnd");      
                }
                // if (msgsnd(msqid, &req, sizeof(req.mtext), 0) == -1) {
                //     perror("msgsnd");
                // }
                break;
            // User wants to upvote a Mik Mak
            case 'U':
                // Get index of Mik Mak to upvote
                voteIndex = req.mtext[2] - '0';
                printf("You want to upvote the MikMak number %d: %s\n",voteIndex, req.mtext);
                // Make sure given index is within bounds
                if (voteIndex <= allMaks.count && voteIndex > 0) {
                    // Print positive version of voteIndex Mik Mak current status
                    if (allMaks.maks[voteIndex - 1].points >= 0)
                        printf("MikMak before upvote: %d. +%d %s\n", voteIndex, allMaks.maks[voteIndex - 1].points, allMaks.maks[voteIndex - 1].text);
                    // Negative version
                    else
                        printf("MikMak before upvote: %d. %d %s\n", voteIndex, allMaks.maks[voteIndex - 1].points, allMaks.maks[voteIndex - 1].text);
                    // Increment points of Mik Mak at voteIndex
                    allMaks.maks[voteIndex - 1].points++;
                    // Print positive version of voteIndex Mik Mak status after upvote
                    if (allMaks.maks[voteIndex - 1].points >= 0)
                        printf("MikMak after upvote: %d. +%d %s\n", voteIndex, allMaks.maks[voteIndex - 1].points, allMaks.maks[voteIndex - 1].text);
                    // Negative version after upvote
                    else
                        printf("MikMak after upvote: %d. %d %s\n", voteIndex, allMaks.maks[voteIndex - 1].points, allMaks.maks[voteIndex - 1].text);
                }
                // Given vote index is out of bounds
                else {
                    printf("That vote index is invalid\n");
                }
                break;
            // User wants to downvote a Mik Mak
            case 'D':
                // Get index of Mik Mak to downvote
                voteIndex = req.mtext[2] - '0';
                printf("You want to downvote the MikMak number %d: %s\n", voteIndex, req.mtext);
                // Make sure given index is within bounds
                if (voteIndex <= allMaks.count && voteIndex > 0) {
                    // Print position version of voteIndex Mik Mak current status
                    if (allMaks.maks[voteIndex - 1].points >= 0)
                        printf("MikMak before downvote: %d. +%d %s\n", voteIndex, allMaks.maks[voteIndex - 1].points, allMaks.maks[voteIndex - 1].text);
                    // Negative version
                    else
                        printf("MikMak before downvote: %d. %d %s\n", voteIndex, allMaks.maks[voteIndex - 1].points, allMaks.maks[voteIndex - 1].text);
                    // Decrement points of Mik Mak at voteIndex
                    allMaks.maks[voteIndex - 1].points--;
                    // Print positive version of voteIndex Mik Mak status after downvote
                    if (allMaks.maks[voteIndex - 1].points >= 0)
                        printf("MikMak after downvote: %d. +%d %s\n", voteIndex, allMaks.maks[voteIndex - 1].points, allMaks.maks[voteIndex - 1].text);
                    // Delete Mik Mak at voteIndex if after vote it is less than -4
                    else if (allMaks.maks[voteIndex - 1].points <= -5) {
                        // Adjust other Mik Maks
                        for (i = voteIndex - 1; i < allMaks.count - 1; i++)
                            allMaks.maks[i] = allMaks.maks[i + 1];
                        // Decrement Mik Mak array count
                        allMaks.count--;
                        printf("Mikmak at index %d was deleted due to having a score below -5\n", voteIndex);
                    }
                    // Negative version after downvote
                    else
                        printf("MikMak after downvote: %d. %d %s\n", voteIndex, allMaks.maks[voteIndex - 1].points, allMaks.maks[voteIndex - 1].text);
                }
                // Given vote index is out of bounds
                else
                    printf("That vote index is invalid\n");
                break;
            // User wants to create a Mik Mak
            case 'S':
                printf("You want to create a MikMak: %s\n", req.mtext);
                // If space for new Mik Mak
                if (allMaks.count < 15) {
                    // Go through the request and parse the Mik Mak text.
                    for (i = 2; i < 53; i++) {
                        allMaks.maks[allMaks.count].text[i - 2] = req.mtext[i];
                    }
                    // Initiate points to 0
                    allMaks.maks[allMaks.count].points = 0;
                    // Increment count of Mik Mak array
                    allMaks.count++;
                    printf("Created the Mik Mak: %d. +%d %s\n", allMaks.count, allMaks.maks[allMaks.count].points, allMaks.maks[allMaks.count - 1].text);
                }
                else {
                    printf("Failed to create Mik Mak because there are already fifteen which is the max.\n");
                }
                break;
            default:
                printf("Request: \"%s\"\n", req.mtext);
        }
    }

    return 0;
}

