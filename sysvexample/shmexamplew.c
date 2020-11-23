#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/errno.h>

// Shared Memory Key
#define SHMKEY 	IPC_PRIVATE

// IPC Permissions (rw-------)
#define PERMS 	0600

// Size of shared memory area
#define SHMSIZE 27

main()
{
    int shmid; // Shared memory identifier
    char *a;
    char *b; 
    int pid,i;
    
    // Get Shared Identifier
    if ((shmid=shmget(SHMKEY,SHMSIZE,PERMS|IPC_CREAT))<0) {
	perror("shmget");
	exit(1);
    }
    // Child - Attach shared memory
    if ((a=(char *)shmat(shmid,NULL,0)) == (char *) -1) {
	perror("shmat");
	exit(1);
    }
    pid = fork();

    if (pid == 0){

	// Put some data in shared memory
	for (i=0;i<SHMSIZE-1;i++) {
	    a[i]='a'+i;
	}
	a[i]='\0';
	printf("Child  : string = %s\n",a); 		

	// Detach segment
	if (shmdt(a)<0) {
	    perror("shmdt");
	    exit(1);
	}
	sleep(2);
	// Wait before terminating
    }
    else {
	sleep(1);
	printf("Parent : string = %s\n",b); 

	// Detach segment
	if (shmdt(b)<0) {
	    perror("shmdt");
	    exit(1);
	}
#if 1
	if (shmctl(shmid,IPC_RMID,NULL)<0) {
	    perror("shmctl");
	    exit(1);
	}
	pause();
#endif
    }
    exit(0);	
}
