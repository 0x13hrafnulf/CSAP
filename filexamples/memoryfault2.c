#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define SIZE 10

int main(int argc, char *argv[])
{
    int i,idx;
    int *array;

    if (argc<2) {
	fprintf(stderr,"\nUsage:\n\t%s <idx>\n",argv[0]);
	exit(1);
    }

    if (!(array=malloc(SIZE*sizeof(int)))) {
	perror("malloc");
	exit(1);
    }
    printf("Size:%d\n",SIZE*sizeof(int));
    idx=atoi(argv[1]);
    for (i=0; i<idx; i+=10) array[i]=i;
    printf("Done\n");
    for (i=0; i<idx; i+=10) printf("array[%2d]=%2d\n",i,array[i]); 
    exit(0);
}
