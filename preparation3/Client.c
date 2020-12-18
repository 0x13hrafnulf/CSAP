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
#include <fcntl.h>
#include <sys/stat.h>

#define SIZE 1024  

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define N 4


void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;
    int sockets[N];                       
    struct sockaddr_in servAddr; 
    unsigned short servPort = SERVER_PORT;     
    char *servIP;        
    char *command;              
    char *fileName;                  
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

    int fd;
    pid_t *childs;
    pid_t child_pid;

    if (!(childs=malloc(N*sizeof(pid_t)))) 
    {
        perror("malloc");
        exit(1);
    }
    if(strcmp(argv[1], "put") == 0) 
    {
        fd = open(argv[2], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr,"%s:", fd);
            perror("open");
            exit(1);
        }

        struct stat info;

        if (fstat(fd, &info) == -1) 
        {
            perror("stat");
            close(fd);
        }

        char buffer[SIZE];
        int size = info.st_size;

        //For parsing filename from pathname
        int len = 0;
        char *p = &(fileName[strlen(fileName)-1]);
        while (p != fileName && *p != '/') p--, len++;
        p++;
        //

        snprintf(buffer, sizeof(buffer) , "%s %s %d", command, p, size);


        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithError("socket() failed");


        memset(&servAddr, 0, sizeof(servAddr));     
        servAddr.sin_family      = AF_INET;             
        servAddr.sin_addr.s_addr = inet_addr(servIP);   
        servAddr.sin_port        = htons(servPort); 


        if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
            DieWithError("connect() failed");

        stringLen = strlen(buffer);          

        if (send(sock, buffer, stringLen, 0) != stringLen)
            DieWithError("send() sent a different number of bytes than expected");

        for(int i = 0; i < N; ++i)
        {
            child_pid = fork();
            if (child_pid < 0) {
                perror("Fork");
                exit(1);
            }
            else if (child_pid == 0) 
            {
                int pid = getpid();

                snprintf(buffer, sizeof(buffer) , "%d\n%s", i, p);










                if ((sockets[i] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
                    DieWithError("socket() failed");

                if (connect(sockets[i], (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
                    DieWithError("connect() failed");


                if (send(sockets[i], buffer, stringLen, 0) != stringLen)
                    DieWithError("send() sent a different number of bytes than expected");

                exit(0);
            }
            else {
                childs[i] = child_pid;              
            } 
        }
        
        for(int i = 0; i < N; ++i)
        {
            int status;
            printf("Waiting for ANY child\n");
            if ((child_pid = waitpid(-1,&status,0)) < 0) {
                perror("waitpid");
            }
            printf ("Child:%d returned %d\n", child_pid, WEXITSTATUS(status));
        }
        free(childs);
    }





    /*
    totalBytesRcvd = 0;
    printf("Received: ");                
    
    while (totalBytesRcvd < stringLen)
    {
        
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");
        totalBytesRcvd += bytesRcvd;   
        echoBuffer[bytesRcvd] = '\0';  
        printf(echoBuffer);            
    }
    */
    printf("\n");    

    close(sock);
    close(fd);
    exit(0);
}