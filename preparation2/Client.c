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

int imdone = 0;

void handler(int sig);
mailbox *getfreembox();

int main(int argc, char *argv[]) 
{
  FILE *fp;

  if (argc != 2) {
    fprintf(stderr, "Usage:\n\t%s <filename>\n", argv[0]);
    exit(0);
  }

  if (strlen(argv[1]) > FILENAMELEN) {
    fprintf(stderr, "File Name too long.\n");
    exit(0);
  }

  fp = fopen(argv[1], "r");
  if (!fp) {
    perror("fopen");
    exit(0);
  }
  fclose(fp);

  if (getsem(SEMPERM) < 0 || initshm(SHMPERM) < 0) {
    fprintf(stderr,"Unable to access shared resources, perhaps server is not running?\n");
    exit(0);
  }

  if (p(FREEBOXES) < 0) {
    exit(0);
  }
  if (p(MBOXLOCK) < 0) {
    exit(0);
  }

  mailbox *mbox = getfreembox();
  if (!mbox) {
    fprintf(stderr, "No free mbox?\n");
    exit(0);
  }

  strncpy(mbox->filename, argv[1], FILENAMELEN);
  bzero(mbox->md5, MD5LEN);
  mbox->pid    = getpid();
  mbox->status = waiting;
  if (v(MBOXLOCK) < 0) {
    exit(0);
  }

  imdone = 0;
  if (signal(SIGUSR1, handler) < 0) {
    perror("signal");
    exit(0);
  }
  if (v(MBOXWAITING) < 0) {
    exit(0);
  }

  while (!imdone) {
    pause();
  }
  if (p(MBOXLOCK) < 0) {
    exit(0);
  }
  if (mbox->status != done) {
    fprintf(stderr, "Mailbox status?\n");
    exit(0);
  }

  char md5[MD5LEN];
  strncpy(md5, mbox->md5, MD5LEN);

  bzero(mbox->md5, MD5LEN);
  bzero(mbox->filename, FILENAMELEN);
  mbox->status = available;

  if (v(MBOXLOCK) < 0) {
    exit(0);
  }
  if (v(FREEBOXES) < 0) {
    exit(0);
  }
  fprintf(stdout, "Filename:%40s\t-\tMd5:%33s\n", argv[1], md5);
    
    
    
    return 0;
}


void handler(int sig)
{
  if (sig == SIGUSR1) {
    imdone = 1;
  }
}

mailbox *getfreembox()
{
  for (int i = 0; i < NUMBOXES; i++) {
    if (mboxes[i].status == available) {
      return(&mboxes[i]);
    }
  }
  return(NULL);
}