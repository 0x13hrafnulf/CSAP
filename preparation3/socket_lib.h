#ifndef MIDTERM3_H
#define MIDTERM3_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 


void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}
  
void HandleTCPClient(int clntSocket, int size, char *buffer) 
{    
    int recvMsgSize;                    

    memset(buffer, 0, size);
    
    if ((recvMsgSize = recv(clntSocket, buffer, size, 0)) < 0)
        DieWithError("recv() failed");


    while (recvMsgSize > 0)      
    {  
        //if (send(clntSocket, Buffer, recvMsgSize, 0) != recvMsgSize)
        //   DieWithError("send() failed");

        if ((recvMsgSize = recv(clntSocket, buffer, size, 0)) < 0)
            DieWithError("recv() failed");
    }

  close(clntSocket);      
}  
void HandleTCPClientGet(int clntSocket, int size, char *buffer) 
{    
    int recvMsgSize;                    

    memset(buffer, 0, size);
    
    if ((recvMsgSize = recv(clntSocket, buffer, size, 0)) < 0)
        DieWithError("recv() failed");


    while (recvMsgSize > 0)      
    {  
        //if (send(clntSocket, Buffer, recvMsgSize, 0) != recvMsgSize)
        //   DieWithError("send() failed");

        if ((recvMsgSize = recv(clntSocket, buffer, size, 0)) < 0)
            DieWithError("recv() failed");
    }
     
}  

int CreateTCPServerSocket(unsigned short port) 
{
    int sock;                        
    struct sockaddr_in ServAddr; 

    
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    
    memset(&ServAddr, 0, sizeof(ServAddr));   
    ServAddr.sin_family = AF_INET;                
    ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ServAddr.sin_port = htons(port);              

    
    if (bind(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
        DieWithError("bind() failed");

    if (listen(sock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    return sock;
}

int AcceptTCPConnection(int servSock) 
{
    int clntSock;                    
    struct sockaddr_in ClntAddr; 
    unsigned int clntLen;            
  
    clntLen = sizeof(ClntAddr);
    
  
    if ((clntSock = accept(servSock, (struct sockaddr *) &ClntAddr, 
           &clntLen)) < 0)
        DieWithError("accept() failed");
    
    printf("Handling client %s\n", inet_ntoa(ClntAddr.sin_addr));

    return clntSock;
} 


#endif