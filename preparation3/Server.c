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


#include "prep_lib.h"

#define N 4
#define SERVER_PORT 12345
#define SIZE 1024

void store_file(int servSock);
int init_storage();
ssize_t parse (int fd, char *buf, size_t sz, off_t *offset);
box *getfreebox();
box *getbox(char *name);
void handler(int sig);
ssize_t readchunk (int fd, char *buf, size_t sz, off_t *offset, size_t chunk);

int main(int argc, char *argv[]) 
{
    int servSock;  
    int clntSock;                      
    unsigned short servPort = SERVER_PORT;           

    box *current_box;
    int semvals[NUMSEM];

    semvals[FREEBOXES] = NUMBOXES;
    semvals[BOXLOCK] = 1;
    semvals[BOXWAITING] = 0;

    if (getsem(SEMPERM | IPC_CREAT) < 0) {
      exit(0);
    }

    if (initsem(semvals) < 0) {
      exit(0);
    }
    if (initshm(SHMPERM | IPC_CREAT) < 0) {
      exit(0);
    }

    if (signal(SIGINT, handler) < 0) {
      perror("signal");
      exit(0);
    } 

    servSock = CreateTCPServerSocket(servPort);

    pid_t *childs;
    pid_t child_pid;

    if (!(childs=malloc(N*sizeof(pid_t)))) 
    {
        perror("malloc");
        exit(1);
    }

    char buffer[SIZE];
    init_storage();


    int fd;

    for (;;) 
    {
      clntSock = AcceptTCPConnection(servSock);
      HandleTCPClient(clntSock, SIZE, buffer);

      printf("%s\n", buffer);

      char fileName[FILENAMELEN]; 
      char *parsed_string;
      parsed_string = strtok(buffer, " ");
      if(strcmp(parsed_string, "put") == 0)
      {
        parsed_string = strtok (NULL, " ");
        snprintf(fileName, sizeof(fileName), "%s", parsed_string);
        parsed_string = strtok (NULL, " ");
        int size = atoi(parsed_string);

        current_box = getfreebox();
        strncpy(current_box->filename, fileName, FILENAMELEN);
        strncpy(current_box->path, "./storage", FILENAMELEN);
        current_box->status = processed;
        current_box->size = size;
        current_box->offset = 0;

        snprintf(buffer, sizeof(buffer) , "%s/%s", current_box->path, current_box->filename);
        
        fd = open(buffer, O_RDWR|O_CREAT, S_IRWXU);
        if (fd<0) {
          fprintf(stderr,"%s:",fd);
          perror("open");
          exit(1);
        }
        current_box->fd = fd;

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
            int pid = getpid();

            //printf("Child process: %d\n", (unsigned int) getpid());
            clntSock = AcceptTCPConnection(servSock);
            HandleTCPClient(clntSock, SIZE, buffer);
            parsed_string = strtok(buffer, "\n");
            int offset = atoi(parsed_string);
            

            char *ptr = buffer;
            ptr += strlen(parsed_string) + 1;

            //printf("Offset:%d\n%s\n", offset, ptr);
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
          child_pid = waitpid(-1,&status,0);
          printf("Waiting for ANY child\n");
          if (child_pid < 0){
              perror("waitpid");
          }
          else if (child_pid == 0) break;
          else ++i;
          printf ("Child:%d returned %d\n", child_pid, WEXITSTATUS(status));
        }
        close(fd);
        current_box->fd = 0;
        free(childs);
       
      }
      else if(strcmp(parsed_string, "get") == 0)
      {
        parsed_string = strtok (NULL, " ");
        snprintf(fileName, sizeof(fileName), "%s", parsed_string);

        current_box = getbox(fileName);
        snprintf(buffer, sizeof(buffer) , "%s/%s", current_box->path, current_box->filename);
        
        fd = open(buffer, O_RDONLY);
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
        int chunk = size / N;

        printf("Filename: %s, size: %d\n", current_box->filename, current_box->size);

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
            int pid = getpid();

            printf("Child process: %d\n", (unsigned int) getpid());
            clntSock = AcceptTCPConnection(servSock);
            HandleTCPClientGet(clntSock, SIZE, buffer);
            parsed_string = strtok(buffer, "\n");
            int part = atoi(parsed_string);            

            off_t offset = part*chunk;
            ssize_t len = 0;

            if(part == N-1) chunk = size - N*chunk;

            len = readchunk(fd, buffer, SIZE, &offset, chunk);
            printf("%s\n", buffer);

            if (send(clntSock, buffer, SIZE, 0) != strlen(buffer))
               DieWithError("send() failed");

            close(clntSock); 
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
          child_pid = waitpid(-1,&status,0);
          printf("Waiting for ANY child\n");
          if (child_pid < 0){
              perror("waitpid");
          }
          else if (child_pid == 0) break;
          else ++i;
          printf ("Child:%d returned %d\n", child_pid, WEXITSTATUS(status));
        }
        free(childs);





      }

    }


    
    return 0;
}



void store_file(int servSock)
{
    int clntSock;                  

    for (;;) 
    {
        clntSock = AcceptTCPConnection(servSock);
        printf("with child process: %d\n", (unsigned int) getpid());
        //HandleTCPClient(clntSock, SIZE,);
    }
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

box *getfreebox()
{
  for (int i = 0; i < NUMBOXES; i++) {
    if (storage[i].status == available) {
      return(&storage[i]);
    }
  }
  return(NULL);
}
box *getbox(char* name)
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
    removesem();
    removeshm();
    exit(0);
  }
}

ssize_t readchunk (int fd, char *buf, size_t sz, off_t *offset, size_t chunk)
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