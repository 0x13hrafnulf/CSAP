#ifndef SOCKETLIB_H
#define SOCKETLIB_H

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
#include <sys/sendfile.h>
#define MAXPENDING 5 

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void send_handle(int a_socket, int size, char *buffer) 
{    
    if (send(a_socket, buffer, size, 0) != size)
        DieWithError("send() sent a different number of bytes than expected");
     
}

void recv_handle(int a_socket, int size, char *buffer) 
{    
    int recv_size;                    

    memset(buffer, 0, size);
    
    if ((recv_size = recv(a_socket, buffer, size, 0)) < 0)
        DieWithError("recv() failed");


    while (recv_size > 0)      
    {  
        if ((recv_size = recv(a_socket, buffer, size, 0)) < 0)
            DieWithError("recv() failed");
    }
 printf("Recv handle: %s\n", buffer);
  close(a_socket);      
}

void recv_handle_no_close(int a_socket, int size, char *buffer) 
{    
    int recv_size;                    

    memset(buffer, 0, size);
    
    if ((recv_size = recv(a_socket, buffer, size, 0)) < 0)
        DieWithError("recv() failed");


    while (recv_size > 0)      
    {  
        if ((recv_size = recv(a_socket, buffer, size, 0)) < 0)
            DieWithError("recv() failed");
    }
     printf("Recv handle no close: %s\n", buffer);
     
}  
void recv_handle_with_reply(int a_socket, int size, char *buffer, char* reply_buffer) 
{    
   int recv_size;                    

    memset(buffer, 0, size);
    
    if ((recv_size = recv(a_socket, buffer, size, 0)) < 0)
        DieWithError("recv() failed");

    send_handle(a_socket, size, reply_buffer);
    while (recv_size > 0)      
    {  
        if ((recv_size = recv(a_socket, buffer, size, 0)) < 0)
            DieWithError("recv() failed");

    }


  close(a_socket);   
     
}  



void send_handle_N(int a_socket, int size, char *buffer, struct sockaddr_in *an_addr) 
{    
    
    if ((a_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    if (connect(a_socket, (struct sockaddr *) an_addr, sizeof(*an_addr)) < 0)
        DieWithError("connect() failed");


    if (send(a_socket, buffer, size, 0) != size)
        DieWithError("send() sent a different number of bytes than expected");
     
}  
void send_handle_N_with_reply(int a_socket, int size, char *buffer, struct sockaddr_in *an_addr) 
{    
    
    if ((a_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    if (connect(a_socket, (struct sockaddr *) an_addr, sizeof(*an_addr)) < 0)
        DieWithError("connect() failed");


    if (send(a_socket, buffer, size, 0) != size)
        DieWithError("send() sent a different number of bytes than expected");
    
    int recv_size;     
    int total = 0;
    while (total < size)
    {
        if ((recv_size = recv(a_socket, buffer, size, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");
        total += recv_size; 
    }
    //recv_handle(a_socket, size, buffer);    
     
} 

int accept_tcp_connection(int a_socket, struct sockaddr_in *an_addr) 
{
    int client_socket;                    
    unsigned int client_len;            
  
    client_len = sizeof(*an_addr);
    
  
    if ((client_socket = accept(a_socket, (struct sockaddr *) an_addr, 
           &client_len)) < 0)
        DieWithError("accept() failed");
    
    printf("Handling client %s\n", inet_ntoa(an_addr->sin_addr));
    printf("I am here\n");
    return client_socket;
} 

int create_tcpserver_socket(unsigned short port, struct sockaddr_in *an_addr) 
{
    int server_sock;                        

    
    if ((server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    
    memset(an_addr, 0, sizeof(*an_addr));   
    an_addr->sin_family = AF_INET;                
    an_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    an_addr->sin_port = htons(port);              

    
    if (bind(server_sock, (struct sockaddr *) an_addr, sizeof(*an_addr)) < 0)
        DieWithError("bind() failed");

    if (listen(server_sock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    return server_sock;
}
int establish_connection(struct sockaddr_in *an_addr, char *an_ip, unsigned short a_port) 
{
    int a_socket;

    if ((a_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithError("socket() failed");


        memset(an_addr, 0, sizeof(*an_addr));     
        an_addr->sin_family      = AF_INET;             
        an_addr->sin_addr.s_addr = inet_addr(an_ip);   
        an_addr->sin_port        = htons(a_port); 

    if (connect(a_socket, (struct sockaddr *) an_addr, sizeof(*an_addr)) < 0)
        DieWithError("connect() failed");
    
    return a_socket;
}


#endif