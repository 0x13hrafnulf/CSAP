#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 10

int main(int argc, char *argv[])
{
    int i,idx;
    int array[SIZE];
    int pippo;

    if (argc<2) {
	fprintf(stderr,"\nUsage:\n\t%s <idx>\n",argv[0]);
	exit(1);
    }
    pippo=3;
    idx=atoi(argv[1]);
    for (i=0; i<idx; i++) array[i]=i;
    printf("Done\n");
    for (i=0; i<idx; i++) printf("array[%2d]=%2d\n",i,array[i]); 
    printf("Pippo:%d\n",pippo);
    exit(0);
}
