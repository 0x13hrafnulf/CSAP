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

#include "socket_lib.h"

#define SIZE 1024  

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define N 4

ssize_t read_chunk (int fd, char *buf, size_t sz, off_t *offset,  size_t chunk);

int main(int argc, char *argv[])
{
    int client_socket;
    int server_socket;
    int client_sockets[N];                       
    struct sockaddr_in server_addr; 
    struct sockaddr_in client_addr; 
    unsigned short server_port = SERVER_PORT;   

    char *server_ip;        
    char *command;              
    char *filename;                  
    unsigned int buffer_len;      
    char buffer[SIZE];

    if ((argc < 3) || (argc > 4))   
    {
       fprintf(stderr, "Usage: %s put/get [filename] \n",
               argv[0]);
       exit(1);
    }

    command = argv[1];  
    server_ip = SERVER_IP;            
    filename = argv[2];         

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

        int size = info.st_size;
        int chunk = size / N;
        //For parsing filename from pathname
        int len = 0;
        char *ptr = &(filename[strlen(filename)-1]);
        while (ptr != filename && *ptr != '/') ptr--, len++;
        ptr++;
        //

        snprintf(buffer, sizeof(buffer) , "%s %s %d", command, ptr, size);

        client_socket = establish_connection(&server_addr, server_ip, server_port);

        buffer_len = strlen(buffer);          

        send_handle(client_socket, buffer_len, buffer);
        //client_socket = establish_connection(&server_addr, server_ip, server_port);
        //sendfile(client_socket, fd, 0, size);

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
                off_t offset = i*chunk;
                char readbuf[SIZE];
                ssize_t len = 0;

                if(i == N-1) chunk = size - i*chunk;

                len = read_chunk(fd, readbuf, SIZE, &offset, chunk);

                snprintf(buffer, sizeof(buffer) , "%d\n%s", i, readbuf);
                printf("Offset:%d len:%d %s\n", chunk, len, readbuf);
 
                send_handle_N(client_socket, SIZE, buffer, &server_addr);
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
    else if(strcmp(argv[1], "get") == 0)
    {
        snprintf(buffer, sizeof(buffer) , "%s %s", command, filename);

        client_socket = establish_connection(&server_addr, server_ip, server_port);

        buffer_len = strlen(buffer);          

        send_handle(client_socket, buffer_len, buffer);
        printf("%s\n", buffer);
        
        fd = open(filename, O_RDWR|O_CREAT, S_IRWXU);
        if(fd < 0) 
        {
          fprintf(stderr,"%s:", fd);
          perror("open");
          exit(1);
        }
        char *parsed_string;
        for(int i = 0; i < N; ++i)
        {
          child_pid = fork();
          if (child_pid < 0) 
          {
              perror("Fork");
              exit(1);
          }
          else if (child_pid == 0) 
          {
            snprintf(buffer, sizeof(buffer) , "%d\n", i);
            printf("%d waits\n", i);
            send_handle_N_with_reply(client_sockets[i], SIZE, buffer, &server_addr);
            parsed_string = strtok(buffer, "\n");
            int offset = atoi(parsed_string);
            
            char *ptr = buffer;
            ptr += strlen(parsed_string) + 1;
            printf("Offset:%d\n%s\n", offset, ptr);
         

            exit(0);
          }
          else 
          {
              childs[i] = child_pid;              
          } 
        }
        
        for(int i = 0; i < N;)
        {
          int status;
          child_pid = waitpid(-1, &status, 0);
          printf("Waiting for ANY child\n");
          if (child_pid < 0)
          {
              perror("waitpid");
          }
          else if (child_pid == 0) break;
          else ++i;
          printf ("Child:%d returned %d\n", child_pid, WEXITSTATUS(status));
        }
        free(childs);
        
    }


    close(client_socket);
    close(fd);
    exit(0);
}


ssize_t read_chunk(int fd, char *buf, size_t sz, off_t *offset, size_t chunk)
{

    ssize_t nchr = 0;
    ssize_t idx = 0;
    char *p = NULL;

    /* position fd & read line */
    if ((nchr = lseek (fd, *offset, SEEK_SET)) != -1)
        nchr = read (fd, buf, chunk);

    if (nchr == -1) {   /* read error   */
        fprintf (stderr, "%s() error: read failure in '%d'.\n",
                __func__, fd);
        return nchr;
    }

    /* end of file - no chars read
    (not an error, but return -1 )*/
    if (nchr == 0) return -1;

    p = buf;    /* check each chacr */
    while (idx < nchr) p++, idx++;
    *p = 0;

    if (idx == nchr) {  /* newline not found  */
        *offset += nchr;

        /* check file missing newline at end */
        return nchr < (ssize_t)chunk ? nchr : 0;
    }

    *offset += idx;

    return idx;
}