#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t *pids,t;
    int nchilds,i;

    if (argc!=2 || (nchilds=atoi(argv[1]))<0) {
	fprintf(stderr,"Usage:\n\t%s <nchild>\n",argv[0]);
	exit(1);
    }

    if (!(pids=malloc(nchilds*sizeof(pid_t)))) {
	perror("malloc");
	exit(1);
    }

    for (i=0; i<nchilds; i++) {
	t=fork();
	if (t<0) {
	    perror("fork");
	    exit(1);
	}
	else if (t==0) {
	    int sl;

	    srand(getpid());
	    sl=((double)rand()/RAND_MAX)*5;
	    sleep(sl);
	    exit(sl);
	}
	else {
	    pids[i]=t;
	}
    }

    i=0;
    do {
	int t,status;

	printf("now waiting for ANY child\n");
	if ((t=waitpid(-1,&status,WNOHANG))<0) {
	    perror("waitpid");
	}
	if (t>0) {
	    printf ("Child:%d returned %d\n",t,WEXITSTATUS(status));
	    i+=1;
	}
	else {
	    sleep(1);
	}
    } while (i<nchilds);
}
