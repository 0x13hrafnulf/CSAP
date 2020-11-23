#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void printdata(int *data,int n,int pid)
{
    int i;

    for (i=0; i<n; i++) {
	printf("%d - data[%d]=%d\n",pid,i,data[i]);
    }
    printf("\n");
}

void changedata(int *data,int n,int v)
{
    int i;

    for (i=0; i<n; i++) {
	data[i]=v;
    }
}

main(int argc, char *argv[])
{
    int data[]={1,2,3,4,5};
    int i,t,pid;
    const int size=sizeof(data)/sizeof(int);

    pid=getpid();
    printdata(data,size,pid);

    printf("Now forking a new process\n");
    
    switch (t=fork()) {
	case 0:
	    pid=getpid();
	    printdata(data,size,pid);
	    sleep(5);
	    printf("%d: Changing values\n",pid);
	    changedata(data,size,getpid());
	    printdata(data,size,pid);
	    break;
	case -1:
	    perror("fork:");
	    exit(1);
	default:
	    printdata(data,size,pid);
	    sleep(5);
	    printf("%d: Changing values\n",pid);
	    changedata(data,size,getpid());
	    printdata(data,size,pid);
	    break;
    }
}
