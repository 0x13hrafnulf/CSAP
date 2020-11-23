#ifndef MIDTERM3_H
#define MIDTERM3_H

#define FILENAMELEN    256
#define MD5LEN         33

typedef enum mb_status { available, waiting, done } mb_status;

typedef struct mbox {
  char      filename[FILENAMELEN]; // it cannot be a pointer (!) 
  char      md5[MD5LEN];
  pid_t     pid;
  mb_status status;
} mailbox;

mailbox *mboxes;

int semid;
int shmid;

#define SEMPERM        0600
#define SHMPERM        0600

#define SHMKEY         0x12345
#define SEMKEY         0x54321

#define MBOXSIZE       sizeof(mailbox)
#define NUMBOXES       10

// Size of shared memory area
#define SHMSIZE        (NUMBOXES * MBOXSIZE)

#define MBOXLOCK       0
#define MBOXWAITING    1
#define FREEBOXES      2

#define LASTSEM        FREEBOXES
#define NUMSEM         (LASTSEM + 1)


int getsem();
int initsem(int vals[]);
int removesem();
int p(int semnum);
int v(int semid);
int initshm();
int removeshm();
mailbox *getfreembox();

#endif
