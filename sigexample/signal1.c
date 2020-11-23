#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void handler(int sig)
{
    pid_t t;
    int status;

    t=wait(&status);
    printf("Child %d exited with status %d\n",t,WEXITSTATUS(status));
}

int main(int argc, char *argv[])
{
    pid_t pid;
    int processes;
    int i,alive;
    pid_t *pids;

    if (argc!=2) {
	fprintf(stderr,"\nUsage\t%s <nchilds>\n",argv[0]);
	exit(1);
    }

    processes=atoi(argv[1]);
    signal(SIGCHLD,handler);

    if (!(pids=malloc(processes*sizeof(pid_t)))) {
	perror("malloc");
	exit(1);
    }

    for (i=0; i<processes; i++) {
	int t,s;

	switch (pids[i]=fork())  {
	    case 0:
		s=getpid();
		srand(s);
		t=(double)rand()/RAND_MAX*10+1; // Race!!!
		printf("Child %d: sleeping for %d sec.\n",s,t);
		sleep(t);
		exit(0);
	    case -1:
		perror("fork");
		exit(1);
	    default:
		break;
	}
    }

    do {
	alive=0;
	printf("sleeping\n");
	sleep(1);
	for (i=0; i<processes; i++) {
	    if (kill(pids[i],0)) {
		printf("%d is dead\n",pids[i]);
	    }
	    else {
		printf("%d is alive\n",pids[i]);
		alive=1;
	    }
	}
    } while (alive);
}
