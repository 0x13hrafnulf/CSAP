#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 


#include "prep_lib.h"

#define N 4
#define SERVER_PORT 12345

void store_file(int servSock);
int init_storage();

int main(int argc, char *argv[]) 
{
    int servSock;  
    int clntSock;                      
    unsigned short servPort = SERVER_PORT;     
    pid_t processID;                 
    unsigned int processLimit = N;       
    unsigned int processCt;          

    servSock = CreateTCPServerSocket(servPort);
    
    




    
    return 0;
}

void store_file(int servSock)
{
    int clntSock;                  

    for (;;) 
    {
        clntSock = AcceptTCPConnection(servSock);
        printf("with child process: %d\n", (unsigned int) getpid());
        HandleTCPClient(clntSock);
    }
}

int init_storage()
{
  for (int i; = 0; i < NUMBOXES; i++) {
    storage[i].status = available;
    bzero(storage[i].filename, FILENAMELEN);
    storage[i].pid = 0;
  }
  return(0);
}