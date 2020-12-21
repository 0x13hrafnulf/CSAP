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
#include "sem_lib.h"

#define N 4
#define SERVER_PORT 12345
#define SIZE 1024

int init_storage();
ssize_t parse (int fd, char *buf, size_t sz, off_t *offset);
box *get_freebox();
box *get_box(char *name);
void handler(int sig);
ssize_t read_chunk (int fd, char *buf, size_t sz, off_t *offset, size_t chunk);

int main(int argc, char *argv[]) 
{
    int server_socket;  
    int client_socket;   
    int server_sockets[N];                    
    struct sockaddr_in server_addr; 
    struct sockaddr_in client_addr; 
    unsigned short server_port = SERVER_PORT;           

    box *current_box;
    int semvals[NUMSEM];

    semvals[FREEBOXES] = NUMBOXES;
    semvals[BOXLOCK] = 1;
    semvals[BOXWAITING] = 0;

    int fd;
    char buffer[SIZE];
    pid_t *childs;
    pid_t child_pid;

    if (!(childs=malloc(N*sizeof(pid_t)))) 
    {
        perror("malloc");
        exit(1);
    }

    if (get_semaphore(SEMPERM | IPC_CREAT) < 0) {
      exit(0);
    }

    if (init_semaphores(semvals) < 0) {
      exit(0);
    }
    if (init_sharedmemory(SHMPERM | IPC_CREAT) < 0) {
      exit(0);
    }

    if (signal(SIGINT, handler) < 0) {
      perror("signal");
      exit(0);
    } 

    init_storage();


    server_socket = create_tcpserver_socket(server_port, &server_addr);


    for (;;) 
    {
      client_socket = accept_tcp_connection(server_socket, &client_addr);
      recv_handle_no_close(client_socket, SIZE, buffer);

      printf("%s\n", buffer);

      char filename[FILENAMELEN]; 
      char *parsed_string;
      parsed_string = strtok(buffer, " ");


      if(strcmp(parsed_string, "put") == 0)
      {
        //parsed_string = strtok (NULL, " ");
        //parsed_string = strtok (NULL, " ");
        //int size = atoi(parsed_string);
        //client_socket = accept_tcp_connection(server_socket, &client_addr);
        //recv_handle(client_socket, size, buffer);
        current_box = get_freebox();
        parsed_string = strtok (NULL, " ");
        strncpy(current_box->filename, parsed_string, FILENAMELEN);    
        strncpy(current_box->path, "./storage", FILENAMELEN);
        parsed_string = strtok (NULL, " ");
        int size = atoi(parsed_string);
        current_box->status = processed;
        current_box->size = size;
        current_box->offset = 0;
        
        snprintf(buffer, sizeof(buffer) , "%s/%s", current_box->path, current_box->filename);
        
        current_box->fd = open(buffer, O_RDWR|O_CREAT, S_IRWXU);
        if(current_box->fd < 0) 
        {
          fprintf(stderr,"%s:", current_box->fd);
          perror("open");
          exit(1);
        }

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
            client_socket = accept_tcp_connection(server_socket, &client_addr);
            recv_handle(client_socket, SIZE, buffer);
            parsed_string = strtok(buffer, "\n");
            int offset = atoi(parsed_string);
            
            char *ptr = buffer;
            ptr += strlen(parsed_string) + 1;

            for(;;)
            {
              if(current_box->offset == offset)
              {
                //p(BOXLOCK);
                //printf("Offset:%d\n%s\n", current_box->offset, ptr);
                write(current_box->fd, ptr, strlen(ptr));   
                current_box->offset += 1;   
                if(current_box->offset == N)
                {
                  current_box->offset = 0;
                }
                //v(BOXLOCK);
                break;
              }
            }

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
        current_box->status = done;
        close(current_box->fd);
        free(childs);  
        
      }
      else if(strcmp(parsed_string, "get") == 0)
      {    
        parsed_string = strtok (NULL, " ");   
        current_box = get_box(parsed_string);
        snprintf(buffer, sizeof(buffer) , "%s/%s", current_box->path, current_box->filename);
        
        current_box->fd = open(buffer, O_RDONLY);
        if(current_box->fd < 0) 
        {
          fprintf(stderr,"%s:", current_box->fd);
          perror("open");
          exit(1);
        }

        int size = current_box->size;
        int chunk = size / N;
        send_handle(client_socket, SIZE, buffer);
        
        for(int i = 0; i < N; ++i)
        {
            child_pid = fork();
            if (child_pid < 0) {
                perror("Fork");
                exit(1);
            }
            else if (child_pid == 0) 
            {
              
              
              printf("%s\n", buffer);
              off_t offset = i*chunk;
              char readbuf[SIZE];
              ssize_t len = 0;

              if(i == N-1) chunk = size - i*chunk;

              len = read_chunk(current_box->fd, readbuf, SIZE, &offset, chunk);

              snprintf(buffer, sizeof(buffer) , "%d\n%s", i, readbuf);
              printf("Offset:%d len:%d %s\n", chunk, len, readbuf);
              client_socket = accept_tcp_connection(server_socket, &client_addr);
              recv_handle_with_reply(client_socket, SIZE, readbuf, buffer);
              

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

        current_box->status = done;
        close(current_box->fd);

        free(childs); 
        
      }

    }

    remove_semaphores();
    remove_sharedmemory();
    return 0;
}



int init_storage()
{
  for (int i = 0; i < NUMBOXES; i++) {
    storage[i].status = available;
    bzero(storage[i].filename, FILENAMELEN);
    storage[i].pid = 0;
    storage[i].size = 0;
  }
  return(0);
}

box *get_freebox()
{
  for (int i = 0; i < NUMBOXES; i++) {
    if (storage[i].status == available) {
      return(&storage[i]);
    }
  }
  return(NULL);
}
box *get_box(char* name)
{
  for (int i = 0; i < NUMBOXES; i++) {
    if (strcmp(storage[i].filename, name) == 0) {
      return(&storage[i]);
    }
  }
  return(NULL);
}

void handler(int sig)
{
  if (sig == SIGINT) {
    remove_semaphores();
    remove_sharedmemory();
    exit(0);
  }
}

ssize_t read_chunk (int fd, char *buf, size_t sz, off_t *offset, size_t chunk)
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