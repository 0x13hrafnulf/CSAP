#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/errno.h>

// IPC Resource Keys  (both could be IPC_PRIVATE)
#define SHMKEY 	((key_t)7890)
#define SEMKEY 	((key_t)7891)

// Shared memory size
#define SHMSIZE	27

// Number of semaphores
#define NSEMS	2

// IPC Permissions (rw-rw-rw)
#define PERMS 	0666


main()
{
    int shmid,semid;
    char *a=""; // This will point to a shared memory buffer
    char *b=""; // "Normal" buffer (allocate by malloc)
    int pid,i;
    struct sembuf sop;
    
    // Get a shared memory segment
    if ((shmid = shmget(SHMKEY,SHMSIZE,PERMS|IPC_CREAT))<0) {
	perror("shmget");
	exit(1);
    }

    // Get Semaphores
    if ((semid=semget(SEMKEY,NSEMS,PERMS|IPC_CREAT))<0) {
	perror("semget");
	exit(1);
    }

    // Initialize Semaphores
    if (semctl(semid,0,SETVAL,0)<0 || semctl(semid,1,SETVAL,0)<0) {
	perror("semctl");
	exit(1);
    }

    // Now get a buffer and initialize it with some data
    b=malloc(SHMSIZE);
    for (i=0; i<SHMSIZE-1; i++) b[i]='a'+i;
    b[i]='\0';

    switch(pid = fork()) {

    case -1:
	perror("fork");
	exit(1);

    case 0:
	// Child - attach shared memory 
	if ((a=(char *)shmat(shmid,NULL,0)) == (char *) -1) {
	    perror("shmat");
	    exit(1);
	}

	// Wait(parent)
	sop.sem_num=1;
	sop.sem_op=-1;
	sop.sem_flg=0;
	if (semop(semid,&sop,1)) {
	    perror("semop");
	    exit(1);
	}

	//Scramble data in both private and shared memory
	printf("Child (before):A:<%s> B:<%s>\n",a,b);
	for (i=0;i<SHMSIZE-1;i++) {
	    a[i]='a'+i;
	    b[i]='z'-i;
	}
	a[i]='\0';
	b[i]='\0';

	printf("Child  (after):A:<%s> B:<%s>\n",a,b);

	// Detach Shared Memory
	if (shmdt(a)<0) {
	    perror("shmdt");
	    exit(1);
	}

	// Signal(Child)
	sop.sem_num=0;
	sop.sem_op=1;
	if (semop(semid,&sop,1)) {
	    perror("semop");
	    exit(1);
	}
	exit(0);
	
    default:
	// Parent - Attach Shared Memory
	if ((a = (char *)shmat(shmid,NULL,0)) ==(char *) -1) {
	    perror("shmat");
	    exit(1);
	}
	printf("Parent(before):A:<%s> B:<%s>\n",a,b);

	// Signal(Parent)
	sop.sem_num=1;
	sop.sem_op=1;
	sop.sem_flg=0;
	if (semop(semid,&sop,1)) {
	    perror("semop");
	    exit(1);
	}

	// Wait(Child) - Will modify shared area
	sop.sem_num=0;
	sop.sem_op=-1;
	if (semop(semid,&sop,1)) {
	    perror("semop");
	    exit(1);
	}

	printf("Parent (after):A:<%s> B:<%s>\n",a,b);
	if (shmdt(a)<0) {
	    perror("shmdt");
	    exit(1);
	}
    }

// ALWAYS REMOVE IPC RESOURCES
    semctl(semid,0,IPC_RMID);
    shmctl(shmid,IPC_RMID,NULL);
}
