#include <stdio.h>

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

    if (argc<2) {
	fprintf(stderr,"Usage %s <num>\n",argv[0]);
	exit(1);
    }

    accumulate(atoi(argv[1]));

    printf("Sum:%d\n",sum);
}
