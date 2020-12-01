#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "utilities.h"


int waitingReplyFromServer = 0;
char rdbuf[MAX_SIZE];


int main(int argc, char *argv[]) {


    int wrfd, rdfd, numread;

    /* Check if an argument was specified. */


    while (1) {
//        printf("wait: %d\n",waitingReplyFromServer);
        char *messageToSendToServer;
        if(waitingReplyFromServer == 0) {
            memset(rdbuf,'\0',MAX_SIZE);
            messageToSendToServer = calloc(MAX_SIZE, sizeof(char));
        printf("client message: ");
        fgets(messageToSendToServer, MAX_SIZE - 1, stdin);
        if (!messageToSendToServer) {
            perror("allocation error");
            exit(1);
        }
        /* Open the first named pipe for writing */
            wrfd = open(NAMED_PIPE1, O_WRONLY);
            //   Write to the pipe
            write(wrfd, messageToSendToServer, MAX_SIZE);
            free(messageToSendToServer);
           // kill(getpid(), MESSAGE_SENT);

    }

        while(1){
            /* Open the second named pipe for reading */
            rdfd = open(NAMED_PIPE2, O_RDONLY);
                // Read from the pipe
                numread = read(rdfd, rdbuf, MAX_SIZE);
           if(strlen(rdbuf) > 0) {
               printf("server message: %s", rdbuf);
              // printf("wait:%d\n", waitingReplyFromServer);
               waitingReplyFromServer = 0;
               break;
           }
            }
    }
}