#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

/* The semaphore key is an arbitrary long integer which serves as an
   external identifier by which the semaphore is known to any program
   that wishes to use it. */

#define KEY (1492)

int main()
{
    int id; /* Number by which the semaphore is known within a program */
    int val;

    /* The next thing is an argument to the semctl() function. Semctl() 
       does various things to the semaphore depending on which arguments
       are passed. We will use it to make sure that the value of the 
       semaphore is initially 0. */

    union semun {
	int val;
	struct semid_ds *buf;
	ushort * array;
    } argument;

    id = semget(KEY, 1, 0666);

    /* Always check system returns. */

    if(id < 0) {
	perror("Unable to obtain semaphore.");
	exit(0);
    }

    /* Get the value of the number 0 semaphore in semaphore array */

    if((val=semctl(id, 0, GETVAL, NULL))< 0) {
	perror("Cannot get semaphore value.");
    }
    else {
	fprintf(stderr, "Semaphore %d has val:%d.\n", KEY,val);
    }
    exit(0);
}

