#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "midterm3.h"

int getsem(int perms)
{
  if ((semid = semget(SEMKEY, NUMSEM, perms)) < 0) {
    perror("semget");
    return(-1);
  }
  return(0);
}

/* initsem -- semaphore initialization */
int initsem(int vals[])
{
  for (int i = 0; i < NUMSEM; i++) {
    if (semctl(semid, i, SETVAL, vals[i]) < 0) {
      perror("semctl");
      return(-1);
    }
  }
  return(0);
}

/* Remove Semaphore Set */
int removesem()
{
  if (semctl(semid, 0, IPC_RMID) < 0) {
    perror("removesem");
    return(-1);
  }
  return(0);
}

/* p -- semaphore p operation */
int p(int semnum)
{
  struct sembuf p_buf;

  p_buf.sem_num = semnum;
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
int v(int semnum)
{
  struct sembuf v_buf;

  v_buf.sem_num = semnum;
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

int initshm(int perms)
{
  shmid = shmget(SHMKEY, SHMSIZE, perms);
  if (shmid < 0) {
    perror("shmget");
    return(-1);
  }

  mboxes = shmat(shmid, NULL, 0);
  if ((void *)mboxes == (void *)-1) {
    perror("shmat");
    return(-1);
  }
  return(0);
}

int removeshm()
{
  if (shmdt(mboxes) < 0) {
    perror("shmdt");
    return(-1);
  }

  if (shmctl(shmid, IPC_RMID, NULL) < 0) {
    perror("shmctl");
    return(-1);
  }

  return(0);
}
