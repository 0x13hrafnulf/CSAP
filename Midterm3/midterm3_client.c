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
#include <time.h>

#include "socket_lib.h"

#define SIZE 2048  

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 16000


int main(int argc, char *argv[])
{
    int client_socket;
    int server_socket;                    
    struct sockaddr_in server_addr; 
    struct sockaddr_in client_addr; 
    unsigned short server_port = SERVER_PORT;   

    char *server_ip;                      
    unsigned int buffer_len;      
    char buffer[SIZE];

    if ((argc < 3))   
    {
       fprintf(stderr, "Usage: %s x y \n",
               argv[0]);
       exit(1);
    }

    server_ip = SERVER_IP;            

    double *mat;
    double amin=1E39,amax=-1E39;
    if (argc!=3) {
	fprintf(stderr,"Usage:\n\t%s <NunRows> <NumCols>\n",argv[0]);
	exit(1);
    }

    int x = atoi(argv[1]);
    int y = atoi(argv[2]); 

    if (x<=0 || y <=0) {
	fprintf(stderr,"Illegal input parameter(s)\n");
	exit(1);
    }

    int size=x*y*sizeof(double);
    mat=malloc(size);
    if (!mat) {
	perror("malloc");
	exit(1);
    }
    srand((unsigned)time(NULL));
    for (int i=0; i<x; i++) {
	for (int j=0; j<y; j++) {
	    double f=(double)rand()/RAND_MAX;
	    mat[i*y+j] = f;
	    if (f>amax) amax=f;
	    else if (f<amin) amin=f;
	}
    }
    printf("\n=====\nMin: %lf\nMax: %lf\n=====\n",amin,amax);


    snprintf(buffer, sizeof(buffer) , "%d %d", x, y);
    //printf("%s", buffer);

    if ((client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");


    memset(&server_addr, 0, sizeof(server_addr));     
    server_addr.sin_family      = AF_INET;             
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);   
    server_addr.sin_port        = htons(SERVER_PORT); 

    if (connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        DieWithError("connect() failed");



    for (int i=0; i<x; i++) {
	    for (int j=0; j<y; j++) 
        {
            sprintf(buffer,"%s %f", buffer, mat[i*y+j]);
	    }
    }
    buffer_len = strlen(buffer);         

    if (send(client_socket, buffer, SIZE, 0) != SIZE)
        DieWithError("send() sent a different number of bytes than expected");

    int recv_size;                    
    memset(buffer, 0, size);
    
    if ((recv_size = recv(client_socket, buffer, size, 0)) < 0)
        DieWithError("recv() failed");
   
    printf("%s\n", buffer);     
    close(client_socket);
    free(mat);
    exit(0);
}

