#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "functions.h"
#include "utilities.h"

int waitingMessageFromClient = 1;
char buf[MAX_SIZE];




int main(int argc, char *argv[]) {
    char *messageToSendToClient;
//    signal(SIGINT, onExitRequested);

    int rdfd, wrfd, ret_val, numread;

    /* Create the first named - pipe */
    ret_val = mkfifo(NAMED_PIPE1, 0666);

    if ((ret_val == -1) && (errno != EEXIST)) {
        perror("Error creating the named pipe");
        exit(1);
    }

    ret_val = mkfifo(NAMED_PIPE2, 0666);

    if ((ret_val == -1) && (errno != EEXIST)) {
        perror("Error creating the named pipe");
        exit(1);
    }
    while (1) {
       if(waitingMessageFromClient == 0) {
           memset(buf,'\0',MAX_SIZE);
           messageToSendToClient = calloc(MAX_SIZE, sizeof(char));
           printf("server message: ");
           fgets(messageToSendToClient, MAX_SIZE - 1, stdin);

           if (!messageToSendToClient) {
               perror("allocation error");
               exit(1);
           }

           /* Open the second named pipe for writing */
           wrfd = open(NAMED_PIPE2, O_WRONLY);
           write(wrfd, messageToSendToClient, strlen(messageToSendToClient) + 1);
           free(messageToSendToClient);

          // kill(getpid(),MESSAGE_SENT);
       }


   while(1){

           /* Open the first named pipe for reading */
           rdfd = open(NAMED_PIPE1, O_RDONLY);

           /* Read from the first pipe */
           numread = read(rdfd, buf, MAX_SIZE);
       if(strlen(buf) > 0) {
           printf("client message: %s", buf);
           waitingMessageFromClient = 0;
           break;
       }
    }



    }

}
