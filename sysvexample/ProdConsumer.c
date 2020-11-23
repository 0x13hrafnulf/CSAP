#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/errno.h>
#include <sys/time.h>

#define SHMKEY 	IPC_PRIVATE // Shared Memory Key
#define SEMKEY 	IPC_PRIVATE // Semaphore Key 
#define PERMS 	0666          // Permissions for Shared Memory Area

#define BUFFERSIZE	4096  // Work Area size

#define NSEMS		3     // We'll use 3 Semaphores
#define LOCK		0     // First semaphore: access to buffer
#define NITEMS		1     // Second semaphore : # of items in WorkArea
#define FREESPACE	2     // Third Semaphore : free space in WorkArea

#define LOCKIT		-1    // Flag: lock the semaphore
#define UNLOCKIT	1     // Flag: unlock the semaphore


// Global Variables: pointers to data in shared memory area
char *a;		// WorkArea
int *ptrs; 		// Pointers to buffers in workarea (array)
int *sizes;		// Size of pointed buffer (array)
int *last;		// Address of last element in ptrs[] and sizes[]
 
int shmid,semid; 	// Shared memory and semaphore identifiers


// Utilities : Lock and Unlock the mutexes


int BufferLock(int flag)	// Lock access to workarea
{
    struct sembuf sop;

    sop.sem_num=LOCK;
    sop.sem_op=flag;
    sop.sem_flg=0;
    if (semop(semid,&sop,1)) {
    	return(-1);
    }
    else {
	return(0);
    }
}

int FreeSpace(int size)		// Amount of free space in workarea
{
    struct sembuf sop;
    
    sop.sem_num=FREESPACE;
    sop.sem_op=size;
    sop.sem_flg=0;
    if (semop(semid,&sop,1)) {
    	return(-1);
    }
    else {
	return(0);
    }
}

int ItemsAvailable(int nitems)	// Items available to Consumer (In workarea)
{
    struct sembuf sop;
    
    sop.sem_num=NITEMS;
    sop.sem_op=nitems;
    sop.sem_flg=0;
    if (semop(semid,&sop,1)) {
	return(-1);
    }
    else {
	return (0);
    }
}

void Producer()		// The producer process
{
    int curpos,size;			// Offset (in workarea) and size of current buffer
    int i;
    int n=0;				// Loop counter
    int dummy[BUFFERSIZE];		// dummy buffer

    while(n<2000) {

	size= rand() % BUFFERSIZE/4 ;	// size of next buffer -- could be 0
	if (size>0) {
	    // Fill buffer
	    for (i=0; i<size; i++) {
		dummy[i]=n;
	    }
	    
	    printf("Producer - Waiting for room to be available\n");
	    fflush(stdout);

	    if (FreeSpace(-size*sizeof(int))) {	// Wait for free space in workarea
		perror("FreeSpace");
		exit(1);
	    }

	    printf("Producer - Locking the buffer\n");
	    fflush(stdout);

	    if (BufferLock(LOCKIT)) {	// Lock access to buffer
		perror("BufferLock");
		exit(1);
	    }


	    // Copy buffer to shared pool - Round Robin index to arrays
	    if (*last<0) { 
		curpos=0;
	    }
	    else {
		// first available byte = first byte of last buffers + its size
		curpos=ptrs[*last]+sizes[*last]*sizeof(int);
	    }

	    // Record buffer size and pointer (offset) to where it starts
	    *last +=1;
	    sizes[*last]=size;
	    bcopy(dummy,&a[curpos],size*sizeof(int));
	    ptrs[*last]=curpos;


	    printf("Producer : Size %d *Pointer %d Last %d\n",size,curpos,*last);
	    
	    printf("Producer - Unlocking the buffer\n");
	    fflush(stdout);
	    
	    if (BufferLock(UNLOCKIT)) {		// Unlock access to buffer
		perror("BufferUnLock");
		exit(1);
	    }

	    n +=1;	    
	    printf("Producer - %d - Incrementing Item Counter(%d)\n",n,size);
	    fflush(stdout);
	    
	    if (ItemsAvailable(1)) {		// Increment # of available buffers to consumers
		perror("ItemsAvailable");
		exit(1);
	    }
	}
    }
}

void Consumer(int initdelay)
{
    int curpos,size;
    int dummy[BUFFERSIZE];        // dummy buffer

    // Wait for some time before starting consumers
    sleep(initdelay);
    while (1) {

	printf("Consumer%d- Waiting for data\n",getpid());
	fflush(stdout);

	// Wait for data to be available
	if (ItemsAvailable(-1)) {
	    perror("ItemsAvailable");
	    exit(1);
	}
	
	    
	printf("Consumer%d - Locking the buffer\n",getpid());
	fflush(stdout);

	if (BufferLock(LOCKIT)) {	// Lock access to shared area
	    perror("BufferLock");
	    exit(1);
	}
	
	curpos=ptrs[*last]; // Pointer (offset) to first byte
	size=sizes[*last];  // Size of chunk
	bcopy(&a[curpos],dummy,size*sizeof(int));
	*last -=1;

	printf("Consumer : Size %d,*Pointer %d Last%d\n",size,curpos,*last);
	

	printf("Consumer%d - Unlocking the buffer\n",getpid());
	fflush(stdout);
	    
	if (BufferLock(UNLOCKIT)) { 	// Unlock shared area
	    perror("BufferUnLock");
	    exit(1);
	}

	printf("Consumer%d - Releasing space (%d)\n",getpid(),size);
	fflush(stdout);

	// Signal Free Space in shared area
	if (FreeSpace(size*sizeof(int))) {
	    perror("FreeSpace");
	    exit(1);
	}
    }
}


int main(int argc, char *argv[])
{
    int pid,i;
    struct timeval now, later, result;
    int sometime;

    sometime=0;
    if (argc==2) {
	sometime=atoi(argv[1]);
    }
    
    // Get Shared memory identifier
    if ((shmid = shmget(SHMKEY,3*BUFFERSIZE+sizeof(int),PERMS|IPC_CREAT))<0) {
	perror("shmget");
	exit(1);
    }

    // Get Semaphores
    if ((semid=semget(SEMKEY,NSEMS,PERMS|IPC_CREAT))<0) {
	perror("semget");
	exit(1);
    }

    // Initialize Semaphores
    if (semctl(semid,NITEMS,SETVAL,0)<0 || 
	semctl(semid,FREESPACE,SETVAL,BUFFERSIZE)<0 ||
	semctl(semid,LOCK,SETVAL,1)<0) {
	perror("semctl");
	exit(1);
    }

    // Attach Shared Memory Segment
    if ((a = (char *)shmat(shmid,NULL,0)) ==(char *) -1) {
	perror("shmat");
	exit(1);
    }

    // Initialize pointers to array in shared memory
    ptrs=(int *)&a[BUFFERSIZE];
    sizes=(int *)&a[2*BUFFERSIZE];
    last=(int *)&a[3*BUFFERSIZE];

    // index of next buffer to be read/written
    *last=-1;

    // Initialize Arrays in shared memory
    for (i=0; i<BUFFERSIZE/sizeof(int); i++) {
	sizes[i]=0;
	ptrs[i]=0;
    }

    // Utility : create a new process group for an happy ending
    if (setpgrp()<0) {
	perror("setpgrp");
	exit(1);
    }

    switch(pid = fork()) {
	
    case -1:
	perror("fork");
	exit(1);
	
    case 0:
	// Consumer, fork again 
	if ((pid=fork())<0) {
	    perror("fork");
	    exit(1);
	}
	// Two consumers
	Consumer(sometime);
	exit(0);
	
		
    default:
	// Parent - Producer
	// Record Time of Startup
	if (gettimeofday(&now,NULL)) {
	    perror("Child: gettimeofday");
	}

	// Start Producer
	Producer();
	
	// Record Time at the End
	if (gettimeofday(&later,NULL)) {
	    perror("Parent: gettimeofday");
	}
	timersub(&later,&now,&result);

	printf("Parent- Execution took %e seconds.\n",result.tv_sec+result.tv_usec/1e6);
    }

    // Destroy IPC resources
    semctl(semid,0,IPC_RMID);
    shmctl(shmid,IPC_RMID,NULL);

    // Send SIGINT to while process group (including itself)
    kill(0,SIGINT);
}
