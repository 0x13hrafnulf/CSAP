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

#include "prep_lib.h"

#define BUFSIZE 1024
#define MD5EXE     "md5sum"


void handler(int sig);
mailbox *getwaitingmbox();
int initmboxes();
int computehash(mailbox *mbox);


int main(int argc, char *argv[]) 
{
  mailbox *mbox;
  int      semvals[NUMSEM];

  semvals[FREEBOXES]   = NUMBOXES;
  semvals[MBOXLOCK]    = 1;
  semvals[MBOXWAITING] = 0;

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
    
  for (;;) {

    p(MBOXWAITING);

    p(MBOXLOCK);
    mbox = getwaitingmbox();
    v(MBOXLOCK);

    if (computehash(mbox) == 0) {

      p(MBOXLOCK);
      mbox->status = done;
      v(MBOXLOCK);

      kill(mbox->pid, SIGUSR1);
    }
    else {
      kill(mbox->pid, SIGTERM);
    }
  }  
    
    
    return 0;
}


void handler(int sig)
{
  if (sig == SIGINT) {
    removesem();
    removeshm();
    exit(0);
  }
}

mailbox *getwaitingmbox()
{
  int i;

  for (i = 0; i < NUMBOXES; i++) {
    if (mboxes[i].status == waiting) {
      return(&mboxes[i]);
    }
  }
  return(NULL);
}

int initmboxes()
{
  int i;

  for (i = 0; i < NUMBOXES; i++) {
    mboxes[i].status = available;
    bzero(mboxes[i].filename, FILENAMELEN);
    bzero(mboxes[i].md5, MD5LEN);
    mboxes[i].pid = 0;
  }
  return(0);
}

int computehash(mailbox *mbox)
{
  char *arg[3];
  char  buffer[BUFSIZE];
  int   fd[2];
  int   pid;

  arg[0] = MD5EXE;
  arg[1] = strdup(mbox->filename);
  arg[2] = NULL;


  if (pipe(fd) < 0) {
    perror("pipe");
    return(-1);
  }

  switch (pid = fork()) {
  case -1:
    perror("fork");
    return(-1);

  case 0:
    close(fd[0]);
    close(1);
    dup2(fd[1], 1);
    execvp(MD5EXE, arg);
    perror("exec");
    return(-1);

  default:
    close(fd[1]);
    if (read(fd[0], buffer, BUFSIZE) < 0) {
      perror("read");
      return(-1);
    }
    char *p = strtok(buffer, " ");
    if (!p) {
      perror("strtok");
      return(-1);
    }
    close(fd[0]);
    wait(NULL);
    strncpy(mbox->md5, p, MD5LEN);
    break;
  }

  free(arg[1]);
  return(0);
}
