#include <stdio.h>
#include <pthread.h>

int sum=0;

int accumulate(int n)
{
    int i=0;

    for (i=0; i<=n; i++ ) {
	sum +=i;
    }
    return(0);
}

main(int argc, char *argv[])
{
    int i=0;
    pthread_t thread1, thread2;

    if (argc<2) {
	fprintf(stderr,"Usage %s <num>\n",argv[0]);
	exit(1);
    }

    i=atoi(argv[1]);

    if (pthread_create
	(&thread1, NULL, accumulate, (void *) (i/2))
	|| pthread_create (&thread2, NULL, accumulate,
			   (void *) (i-i/2))) {
	perror ("pthread_create");
	exit (1);
    }

    pthread_join (thread1, NULL);
    pthread_join (thread2, NULL);

    printf("Sum:%d\n",sum);
}
