#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int count=0;

void handler(int t)
{
    printf("Ouch\n");
    count +=1;
}

main(int argc, char *argv[])
{
    struct sigaction s;
    sigset_t omask,nmask,psigs;
    int i;

    s.sa_handler=handler;
    sigemptyset(&nmask);
    s.sa_mask=nmask; /* segnali bloccati durante il dispatch */
    s.sa_flags=0;

    if (sigaction(SIGINT,&s,NULL)<0) {
	perror("sigaction");
	exit(1);
    }

    while (count<3) sleep(1);

    sigemptyset(&nmask);
    if (sigaddset(&nmask,SIGINT)) {
	perror("sigaddset");
	exit(1);
    }

    printf("Blocking SIGINT\n");
    if (sigprocmask(SIG_BLOCK,&nmask,&omask)) {
	perror("sigprocmask");
	exit(1);
    }

    for (i=0; i<5; i++) {
	if (!sigpending(&psigs) && sigismember(&psigs,SIGINT)) {
	    printf("SIGINT pending.\n",i);
	}
	sleep(1);
    }

    printf("Restoring sigmask\n");
    if (sigprocmask(SIG_SETMASK,&omask,&nmask)) {
	perror("sigprocmask");
	exit(1);
    }
    while (count<5) sleep(1);
}
