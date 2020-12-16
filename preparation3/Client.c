#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>      
#include <string.h>     
#include <unistd.h>     

#define RCVBUFSIZE 32  

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define N 4

int main(int argc, char *argv[])
{
    int sock;                       
    struct sockaddr_in servAddr; 
    unsigned short servPort = SERVER_PORT;     
    char *servIP;        
    char *command;              
    char *fileName;                
    char echoBuffer[RCVBUFSIZE];     
    unsigned int stringLen;      
    int bytesRcvd, totalBytesRcvd;   

    if ((argc < 3) || (argc > 4))   
    {
       fprintf(stderr, "Usage: %s put/get [filename] \n",
               argv[0]);
       exit(1);
    }

    command = argv[1];  
    servIP = SERVER_IP;            
    fileName = argv[2];         



    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");


    memset(&servAddr, 0, sizeof(servAddr));     
    servAddr.sin_family      = AF_INET;             
    servAddr.sin_addr.s_addr = inet_addr(servIP);   
    servAddr.sin_port        = htons(servPort); 


    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("connect() failed");

    stringLen = strlen(fileName);          

    
    if (send(sock, fileName, echoStringLen, 0) != echoStringLen)
        DieWithError("send() sent a different number of bytes than expected");

    
    totalBytesRcvd = 0;
    printf("Received: ");                
    while (totalBytesRcvd < echoStringLen)
    {
        
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");
        totalBytesRcvd += bytesRcvd;   
        echoBuffer[bytesRcvd] = '\0';  
        printf(echoBuffer);            
    }

    printf("\n");    

    close(sock);
    exit(0);
}