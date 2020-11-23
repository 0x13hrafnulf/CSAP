#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

int i=0;

#define SIG1 SIGUSR1
#define SIG2 SIGUSR2

#define UNBLOCK

void handler1(int sig)
{
    printf("SIG1,%d\n",i);
}

void handler2(int sig)
{
    printf("SIG2,%d\n" ,i);
}

main()
{
    
    signal(SIG1,handler1);
    signal(SIG2,handler2);
    for (;;) { 
	int t,m=siggetmask();
	t=sigmask(SIG1)|sigmask(SIG2)|m;
	if (sigsetmask(t)!=m) {
	    perror("mask");
	    exit(1);
	}

	kill(getpid(),SIG1);
//	kill(getpid(),SIG2);
	kill(getpid(),SIG1);
//	kill(getpid(),SIG2);
	kill(getpid(),SIG1);
//	kill(getpid(),SIG2);
	i +=1;

#if defined(UNBLOCK)
	if (sigsetmask(m)!=t) {
	    perror("unmask");
	    exit(1);
	}
#endif
	sleep(1);
    }
}
