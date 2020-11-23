#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int processes;

void handler(int sig)
{
    pid_t t;
    int status;

    t=wait(&status);
    printf("Child %d exited with status %d\n",t,WEXITSTATUS(status));
    processes -=1;
}

int main(int argc, char *argv[])
{
    pid_t pid;
    int i;

    if (argc!=2) {
	fprintf(stderr,"\nUsage\t%s <nchilds>\n",argv[0]);
	exit(1);
    }

    signal(SIGCHLD,handler);
    processes=atoi(argv[1]);

    for (i=0; i<processes; i++) {
	pid=fork();
	if (pid<0) {
	    perror("fork");
	    exit(1);
	}
	else if (!pid) {
	    int t,s;
	    s=getpid();
	    srand(s);
	    t=(double)rand()/RAND_MAX*10+1; // Race!!!
	    printf("Child %d: sleeping for %d sec.\n",s,t);
	    sleep(t);
	    exit(t);
	}
    }
    while (processes) {
	printf("sleeping\n");
	sleep(1);
    }
}
