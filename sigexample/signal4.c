#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>


#define SIG1 SIGUSR1

#define UNBLOCK 

int sigreceived=0;

void handler1(int sig)
{
    sigreceived++;
}

main(int argc, char *argv[])
{
    int i;
    int t,m;
    m=siggetmask();
    signal(SIG1,handler1);
    t=sigmask(SIG1)|m;
    if (argc>1  && sigsetmask(t)!=m) {
	perror("mask");
	exit(1);
    }

    for (i=0;i<10;i++) { 
	kill(getpid(),SIG1);
    }

    if (argc>1 && sigsetmask(m)!=t) {
	perror("unmask");
	exit(1);
    }
    sleep(1);
    printf("%d\n",sigreceived);
}
