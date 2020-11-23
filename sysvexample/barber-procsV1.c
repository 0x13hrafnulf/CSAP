#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

// The maximum number of customers
#define MAX_CUSTOMERS 25

// Define # of semaphores.
#define NSEMS 4

// waitingRoom Limits the # of customers allowed
// to enter the waiting room at one time.
#define WAITINGROOM 0

// barberChair ensures mutually exclusive access to
// the barber chair.
#define BARBERCHAIR 1

// barberPillow is used to allow the barber to sleep
// until a customer arrives.
#define BARBERPILLOW 2

// seatBelt is used to make the customer to wait until
// the barber is done cutting his/her hair.
#define SEATBELT 3


// Flag to stop the barber thread when all customers
// have been serviced.
int allDone=0; 

//Shared Memory
#define PERMS 	0600
int semid;

int ipcinit(int nsems,int vals[])
{
    void *a;
    int i;

    if ((semid=semget(IPC_PRIVATE,nsems,PERMS|IPC_CREAT))<0) {
	perror("semget\n");
	return(1);
    }

    for (i=0; i<nsems; i++) {
	if (semctl(semid,i,SETVAL,vals[i])<0) {
	    perror("setval:semctl\n");
	    return(1);
	}
    }

    return(0);
}

int ipcend()
{
    int sts=0;

    if (semctl(semid,0,IPC_RMID)) {
	perror("semctl");
	sts=1;
    }
    return(sts);
}

int sempost(int snum)
{
    struct sembuf sop;

    sop.sem_num=snum;
    sop.sem_op=1;
    sop.sem_flg=0;
    if (semop(semid,&sop,1)) {
	perror("post:semop");
	return(-1);
    }
    return(0);
}

int semwait(int snum)
{
    struct sembuf sop;

    sop.sem_num=snum;
    sop.sem_op=-1;
    sop.sem_flg=0;
    if (semop(semid,&sop,1)) {
	perror("wait:semop");
	return(-1);
    }
    return(0);
}

void randwait(int secs) 
{
    int len;
    // Generate a random number...
    len = (int) ((drand48() * secs) + 1);
    sleep(len);
}

void customer(int num) 
{
    printf("Customer %d is going to the barber's shop.\n", num);
    // Take some random amount of time
    randwait(5);

    printf("Customer %d arrived at barber shop.\n", num);
    // Wait for space in the waiting room...
    semwait(WAITINGROOM);

    printf("Customer %d entering waiting room.\n", num);
    // Wait for the barber chair 
    semwait(BARBERCHAIR);

    // The chair is free so give up space in the waiting room.
    sempost(WAITINGROOM);

    // Wake up the barber...
    printf("Customer %d waking the barber.\n", num);
    sempost(BARBERPILLOW);

    // Wait for the barber to cut hair.
    semwait(SEATBELT);

    // Give up the chair.
    sempost(BARBERCHAIR);

    printf("Customer %d leaving barber shop.\n", num);
    exit(0);
}

void theend(int s)
{
    allDone=1;
}

void barber() 
{
   while (allDone==0) {
	// Sleep until customer arrives
	printf("Barber is sleeping\n");
	semwait(BARBERPILLOW);

	if (allDone==0) {
	    printf("Barber is cutting hair\n");

	    // Take a random amount of time 
	    randwait(3);
	    printf("Barber has finished cutting hair.\n");

	    // Release the customer 
	    sempost(SEATBELT);
	}
	else {
	    printf("Barber is going home.\n");
	}
    }
    exit(0);
}

int startbarber()
{ 
    int pid;

    pid=fork();
    switch(pid) {
	case 0:
	    // Child
	    if (signal(SIGUSR1,theend)<0) {
		perror("signal");
	    }
	    barber();
	case -1:
	    perror("fork");
	    return(-1);
	default:
	    // Parent
	    return(pid);
    }
}

int startcustomer(int i)
{
    int pid;

    pid=fork();
    switch(pid) {
	case 0:
	    // Child
	    customer(i);
	case -1:
	    perror("fork");
	    return(-1);
	default:
	    // Parent
	    return(pid);
    }
}


int main(int argc, char *argv[]) 
{
    int i, numCustomers, numChairs;
    pid_t pid[MAX_CUSTOMERS],bpid;
    int initvalues[NSEMS];

    // Check command line arguments.
    if (argc != 3) {
	printf("Use: %s <NCustomers> <NChairs>\n",argv[0]);
	exit(-1);
    }

    numCustomers = atoi(argv[1]);
    numChairs = atoi(argv[2]);

    if (numCustomers > MAX_CUSTOMERS) {
	printf("The maximum number of Customers is %d.\n", MAX_CUSTOMERS);
	exit(-1);
    }
    // Initialize random number generator
    srand48((long)getpid());


    // Setup initial values for semaphores 
    initvalues[WAITINGROOM]=numChairs;
    initvalues[BARBERCHAIR]=1;
    initvalues[BARBERPILLOW]=0;
    initvalues[SEATBELT]=0;

    // Create IPCs
    if (ipcinit(NSEMS,initvalues)) {
	exit(1);
    }

    // Create the barber process
    bpid=startbarber();

    // Create the customer processes.
    for (i=0; i<numCustomers; i++) {
	pid[i]=startcustomer(i);
    }

    // Wait for customer processes to finish
    for (i=0; i<numCustomers; i++) {
	if (waitpid(pid[i],NULL,NULL)<0) {
	    perror("waitpid");
	}
    }

    // Stop the barber thread.

    // Wake the barber (if sleeping)
    sempost(BARBERPILLOW); 
    if (kill(bpid,SIGUSR1)<0) {
	perror("kill");
    }	

    if (waitpid(bpid,NULL,NULL)<0) {
	perror("waitpid");
    }

    // Remove IPCs
    ipcend();
    exit(0);
}

