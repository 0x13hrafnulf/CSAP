#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <errno.h>
#include <stdio.h>
#include <signal.h>

extern int errno;

#define SHMKEY (key_t) IPC_PRIVATE		  /* shared memory key */
#define SKEY   (key_t) IPC_PRIVATE		  /* semaphore key  */
#define SEMPERM 0600				  /* permission for sem */
#define FULL 0					  /* sempahore: full */
#define EMPTY 1					  /* semaphore: empty */
#define MUTEX 2					  /* semaphore: mutex */
#define BUFSIZE 10				  /* buffer size */

/* will hold data and read count */

char buffer[BUFSIZE];


#define IFLAGS (SEMPERM | IPC_CREAT)
#define ERR ((char *) -1)

static int shmid, semid;

/* create and attach shared memory segments */

int getseg(char **buf)
{

    /* create shared memory segments */
    if ((shmid = shmget(SHMKEY, BUFSIZE, IFLAGS)) < 0) {
	perror("shmget failed");
	return(-1);
    }	

    /* attach shared memory segments */
    if ((*buf = (char *) shmat(shmid, 0, 0)) == ERR) {
	perror("shmat failed");
	return(-1);
    }	
}

/* initsem -- semaphore initialization */

int initsem(key_t semkey)
{
    int status = 0;				  /* ret val from init. */
    union semun {
      int val;
      struct semid_ds *stat;
      ushort *array;
    } ctl_arg;

    if ((semid = semget(semkey, 3, IFLAGS))>0) {
	    /* if ok, initialize the semaphores */
	    ushort array[3] = {BUFSIZE, 0, 1};
	    ctl_arg.array = array;
	    status = semctl(semid, 0, SETALL, ctl_arg);
    }


    if (semid == -1 || status == -1) {
	perror("initsem failed");
	return(-1);
    } else
	return(semid);
}

    
/* remove shared memory identifiers and sem set id */
int ipcremove()
{
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
	perror("remove_shm failed");
	return(-1);
    } 
    if (semctl(semid, 0, IPC_RMID) < 0) {
	perror("remove_sem failed");
	return(-1);
    }
}	

/* p -- semaphore p operation */
/* We use 'p' here, rather than more understandable 'wait', since the latter */
/* can be confused with the Unix 'wait'. */

int p(int semid, int semnum)
{

    struct sembuf p = {0,  -1, 0};
   
    p.sem_num=semnum;	
    if (semop(semid, &p, 1) == -1) {
	fprintf(stderr,"p(%d) failed",semnum);
	perror("");
	exit(1);
    } 
    else {
	return(0);
    }
}
   

/* v -- semaphore v operation */

int v(int semid, int semnum)
{

    struct sembuf v={0, 1, 0};	

    v.sem_num=semnum;	
    if (semop(semid, &v, 1) == -1) {
	fprintf(stderr,"v(%d) failed",semnum);
	perror ("");
	exit(1);
    } 
    else {
	return(0);
    }
}

int p0(int semid, int semnum)
{

    struct sembuf p = {0,  0, 0};
   
    p.sem_num=semnum;	
    if (semop(semid, &p, 1) == -1) {
	fprintf(stderr,"p(%d) failed",semnum);
	perror("");
	exit(1);
    } 
    else {
	return(0);
    }
}
   

/* consumer -- consumer program using a bounded buffer implemented */
/* with semaphores: get a character from the buffer and print it out */
int consumer(int semid, char *buffer)
{

    char nextc;
    int out = 0;

    for (;;) {
	p(semid, FULL);				  /* if any data, */
	p(semid, MUTEX);
	nextc = buffer[out];			  /* get it */
	out = (out + 1) % BUFSIZE;
	v(semid, MUTEX);
	v(semid, EMPTY);			  /* signal buffer has space */
	printf("%c", toupper(nextc));
	fflush(stdout);
    }
}


/* producer -- read in a character from stdin and place it in the buffer */
int producer(int semid, char *buffer)
{

    char nextp;
    int in = 0, n;

    for (;;) { 
	/* read a character */
	if ((n=read(0,&nextp,sizeof(nextp)))<=0) {
	    if (n<0) perror("read");
	    break;				  /* if EOF, finish it */	 
	}
	p(semid, EMPTY);			  /* if any space, */
	p(semid, MUTEX);
	buffer[in] = nextp;			  /* add it to buffer */
	in = (in + 1) % BUFSIZE;
	v(semid, MUTEX);
	v(semid, FULL);				  /* signal buffer has data */
    }
    p0(semid,FULL);
}


/* simple server for a semaphore server */

main()
{
    int semid;
    int pid;
    char *buf;

    if ((semid = initsem(SKEY)) < 0)		  /* create a semaphore set */
	exit(1);

    getseg(&buf);				  /* create & attach shm */

    switch (pid = fork()) {
    case -1:
	perror("fork");
	break;
    case 0:					  /* child */
	/* break link with terminal */
	consumer(semid, buf);
	break;					  /* actually, never exits */
    default:					  /* parent */
	producer(semid, buf);
	kill(pid,SIGTERM);
	waitpid(pid,NULL,0);
	ipcremove();
	break;
    }
    exit(pid != -1 ? 0 : 1);
}
