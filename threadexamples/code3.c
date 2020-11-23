#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct { int i,j; } RANGE;

void *accumulate(void *r)
{
    RANGE *n=(RANGE *)r;
    int i=0;
    long sum=0;

    for (i=n->i; i<=n->j; i++ ) {
	sum +=i;
    }
    pthread_exit((void *)sum);
    return NULL;
}


void main(int argc, char *argv[])
{
    int i=0;
    pthread_t thread1, thread2;
    long sum, temp;
    RANGE r1,r2;

    if (argc<2) {
	fprintf(stderr,"Usage %s <num>\n",argv[0]);
	exit(1);
    }

    i=atoi(argv[1]);

    r1.i=0; r1.j=i/2;
    r2.i=i/2+1; r2.j=i;

    if (pthread_create
	(&thread1, NULL, accumulate, (void *) &r1)
	|| pthread_create (&thread2, NULL, accumulate,
			   (void *) &r2)) {
	perror ("pthread_create");
	exit (1);
    }

    pthread_join (thread1,(void *)&sum );
    pthread_join (thread2,(void *)&temp);

    printf("Sum:%d\n",sum+temp);
}
