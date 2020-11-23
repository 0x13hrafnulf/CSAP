#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

extern int errno;

#define SEMPERM 0600

/* initsem -- semaphore initialization */
int initsem (key_t semkey)
{
    int status = 0, semid;
    union semun {		/* this has to be declared */
	int val;
	struct semid_ds *stat;
	ushort *array;
    } ctl_arg;

    if ((semid = semget (semkey, 1, SEMPERM | IPC_CREAT)) > 0) {
	ctl_arg.val = 1;	/* semctl should be called with */
	status = semctl (semid, 0, SETVAL, ctl_arg);
    }

    if (semid < 0 || status < 0) {
	perror ("initsem");
	return (-1);
    }
    else
	return (semid);
}

/* Remove Semaphore Set */
int removesem (int semid)
{
    if (semctl (semid, 0, IPC_RMID) < 0) {
	perror ("removesem");
	return (-1);
    }
    return (0);
}


/* p -- semaphore p operation */

/* We use 'p' here, rather than more understandable 'wait', since the latter */

/* can be confused with the Unix 'wait'. */

int p (int semid)
{

    struct sembuf p_buf;

    p_buf.sem_num = 0;
    p_buf.sem_op = -1;
    p_buf.sem_flg = 0;

    if (semop (semid, &p_buf, 1) == -1) {
	perror ("p(semid) failed");
	return (-1);
    }
    else {
	return (0);
    }
}


/* v -- semaphore v operation */

int v (int semid)
{

    struct sembuf v_buf;

    v_buf.sem_num = 0;
    v_buf.sem_op = 1;
    v_buf.sem_flg = 0;

    if (semop (semid, &v_buf, 1) == -1) {
	perror ("v(semid) failed");
	return (-1);
    }
    else {
	return (0);
    }
}

void dosleep ()
{
    sleep ((int) (drand48 () * 2));
}

void handlesem (int mutex)
{
    int pid = getpid ();
    int i = 0;

    srand48 ((long) pid);
    while (i < 10) {

	printf ("process %d BEFORE critical section\n", pid);

	if (p (mutex)) {
	    break;
	}

	printf (">> process %d IN critical section\n", pid);

	/* in real life, do something interesting */

	dosleep ();

	if (v (mutex)) {
	    break;
	}
	printf ("process %d AFTER critical section\n", pid);
	i += 1;
    }

    printf ("process %d exiting\n", pid);
    exit (0);
}

int main (int argc, char *argv[])
{
    key_t semkey = 0x200;
    int mutex;
    int i, n;

    if (argc != 2 || (n = atoi (argv[1])) < 0) {
	fprintf (stderr, "Usage:\n\t%s <nprocs>\n", argv[0]);
	exit (1);
    }

    if ((mutex = initsem (semkey)) < 0) {
	exit (1);
    }
    for (i = 0; i < n; i++) {
	int p;
	if (!(p = fork ())) {
	    handlesem (mutex);
	}
	else if (p < 0) {
	    perror ("fork");
	    break;
	}
    }
    for (i = 0; i < n; i++) {
	if (wait (NULL) < 0) {
	    perror ("wait");
	    break;
	}
    }
    removesem (mutex);
}
