#ifndef SEMLIB_H
#define SEMLIB_H

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

#define FILENAMELEN    256
#define MD5LEN         33

int semid;
int shmid;

#define SEMPERM        0600
#define SHMPERM        0600

#define SHMKEY         0x12345
#define SEMKEY         0x54321

#define BOXSIZE       sizeof(box)
#define NUMBOXES       10


#define SHMSIZE        (NUMBOXES * BOXSIZE)

#define BOXLOCK       0
#define BOXWAITING    1
#define FREEBOXES     2

#define LASTSEM        FREEBOXES
#define NUMSEM         (LASTSEM + 1)
 

typedef enum box_status { available, processed, done } box_status;

typedef struct box {
  char      filename[FILENAMELEN]; 
  char      path[FILENAMELEN]; 
  pid_t     pid;
  int fd;
  int size; 
  box_status status;
  int offset;
} box;

box* storage;


int get_semaphore(int permission)
{
  if ((semid = semget(SEMKEY, NUMSEM, permission)) < 0) {
    perror("semget");
    return(-1);
  }
  return(0);
}

int init_semaphores(int values[])
{
  for (int i = 0; i < NUMSEM; i++) {
    if (semctl(semid, i, SETVAL, values[i]) < 0) {
      perror("semctl");
      return(-1);
    }
  }
  return(0);
}

int remove_semaphores()
{
  if (semctl(semid, 0, IPC_RMID) < 0) {
    perror("removesem");
    return(-1);
  }
  return(0);
}

int p(int semaphore_num)
{
  struct sembuf p_buf;

  p_buf.sem_num = semaphore_num;
  p_buf.sem_op  = -1;
  p_buf.sem_flg = 0;

  if (semop(semid, &p_buf, 1) == -1) {
    perror("p(semid) failed");
    return(-1);
  }
  else {
    return(0);
  }
}

/* v -- semaphore v operation */
int v(int semaphore_num)
{
  struct sembuf v_buf;

  v_buf.sem_num = semaphore_num;
  v_buf.sem_op  = 1;
  v_buf.sem_flg = 0;

  if (semop(semid, &v_buf, 1) == -1) {
    perror("v(semid) failed");
    return(-1);
  }
  else {
    return(0);
  }
}

int init_sharedmemory(int permission)
{
  shmid = shmget(SHMKEY, SHMSIZE, permission);
  if (shmid < 0) {
    perror("shmget");
    return(-1);
  }

  storage = shmat(shmid, NULL, 0);
  if ((void *)storage == (void *)-1) {
    perror("shmat");
    return(-1);
  }
  return(0);
}

int remove_sharedmemory()
{
  if (shmdt(storage) < 0) {
    perror("shmdt");
    return(-1);
  }

  if (shmctl(shmid, IPC_RMID, NULL) < 0) {
    perror("shmctl");
    return(-1);
  }

  return(0);
}


#endif