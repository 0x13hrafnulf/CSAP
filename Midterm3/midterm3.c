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
#define SERVER_PORT 16000
#define SHMKEY      0x12345
#define SEMKEY         0x54321
int shmid;
int semid;

int init_sharedmemory(int permission, int x, int y);
int remove_sharedmemory();
void handler(int sig);

double *matrix;


double find_min(int index_y, int n_x, int n_y);
double find_max(int index_y, int n_x, int n_y);

int main(int argc, char *argv[]) 
{
    int server_socket;  
    int client_socket;                      
    struct sockaddr_in server_addr; 
    struct sockaddr_in client_addr; 
    unsigned short server_port = SERVER_PORT; 

    char buffer[SIZE];
    pid_t *childs;
    pid_t child_pid;

    if (argc < 2)   
    {
       fprintf(stderr, "Usage: %s <N> \n",
               argv[0]);
       exit(1);
    }


    int N = atoi(argv[1]);  


    server_socket = create_tcpserver_socket(server_port, &server_addr);

    int x = 0;
    int y = 0;

    if (!(childs=malloc(N*sizeof(pid_t)))) 
    {
        perror("malloc");
        exit(1);
    }

    for (;;) 
    {
        client_socket = accept_tcp_connection(server_socket, &client_addr);
        recv_handle_no_close(client_socket, SIZE, buffer);

        char *parsed_string;
        parsed_string = strtok(buffer, " ");
        x = atoi(parsed_string);
        parsed_string = strtok(NULL, " ");
        y = atoi(parsed_string);
        parsed_string = strtok(NULL, " ");
        printf("%d %d\n", x , y);
 
        if (init_sharedmemory(0600 | IPC_CREAT, x ,y ) < 0) {
        exit(0);
        }

        if (signal(SIGINT, handler) < 0) {
            perror("signal");
            exit(0);
        } 

        for(int i = 0; i < x; ++i)
        {
            for(int j = 0; j < y; ++j)
            {

                double val = atof(parsed_string);
                matrix[i*y+j] = val;
                parsed_string = strtok(NULL, " ");
                
            }
        }

        double max = -1E39;
        double min = 1E39;


        int channel[2]; 
        if(pipe(channel) == -1) {    
            perror("pipe call error");
            exit(1);
        }
        char readbuff[256];
        int c = 0;
        for(int i = 0; i < N; ++i)
        {
          if(i < y) 
          {
            child_pid = fork();
            if (child_pid < 0) 
            {
                perror("Fork");
                exit(1);
            }
            else if (child_pid == 0) 
            {
                close(channel[0]);
                double val_min = find_min(i, x, y);
                double val_max = find_max(i, x, y);
                //printf("%f %f\n", val_min, val_max);    
                snprintf(readbuff, 256 , "%f %f", val_min, val_max);
                
                int out = write(channel[1], readbuff, strlen(readbuff)); 
                
                if(out < strlen(readbuff))
                {
                    if (out < 0) perror("Error write in child");
                }

                exit(0);
            }
            else 
            {

                childs[i] = child_pid;              
            }
            
         }
          
        }
        close(channel[1]);
        while ((c = read(channel[0], readbuff, sizeof(readbuff)-1)) != 0) 
        { 
        readbuff[c] = '\0'; 

        parsed_string = strtok(readbuff, " ");
        min = min > atof(parsed_string)? atof(parsed_string) :  min;
        parsed_string = strtok(NULL, " ");
        max = max < atof(parsed_string) ? atof(parsed_string) : max;

        }
        for(int i = 0; i < N;)
        {
          int status;
          child_pid = waitpid(-1, &status, 0);
          if (child_pid < 0)
          {
              perror("waitpid");
          }
          else if (child_pid == 0) break;
          else ++i;
        }

        snprintf(buffer, SIZE , "%f %f", min, max);
        send_handle(client_socket, SIZE, buffer);

        free(childs);  
    }


}

double find_min(int index_y, int n_x, int n_y)
{

    double min = 1E39;

    for(int i = 1; i < n_x; ++i)
    {
        if(min > matrix[n_y*index_y+i])
            min = matrix[n_y*index_y+i];

    }
    return min;
}
double find_max(int index_y, int n_x, int n_y)
{
    double max = -1E39;
    for(int i = 0; i < n_x; ++i)
    {
        if(max < matrix[n_y*index_y+i])
            max = matrix[n_y*index_y+i];
    }  
    return max;
}

int init_sharedmemory(int permission, int x, int y)
{
    shmid = shmget(SHMKEY, x * y * sizeof(double), permission);
    if (shmid < 0) {
    perror("shmget");
    return(-1);
    }

    matrix = shmat(shmid, NULL, 0);
    if ((void *)matrix == (void *)-1) {
    perror("shmat");
    return(-1);
    }
    return(0);
}
int remove_sharedmemory()
{
    if (shmdt(matrix) < 0) {
    perror("shmdt");
    return(-1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
    perror("shmctl");
    return(-1);
    }

    return(0);

}

void handler(int sig)
{
    if (sig == SIGINT) {
    remove_sharedmemory();
    exit(0);
    }
}

