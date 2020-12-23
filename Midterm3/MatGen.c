#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

void main(int argc, char *argv[])
{
    double *mat;
    double amin=1E39,amax=-1E39;
    if (argc!=3) {
	fprintf(stderr,"Usage:\n\t%s <NunRows> <NumCols>\n",argv[0]);
	exit(1);
    }
    int sizex=atoi(argv[1]);
    int sizey=atoi(argv[2]);
    if (sizex<=0 || sizey <=0) {
	fprintf(stderr,"Illegal input parameter(s)\n");
	exit(1);
    }

    int size=sizex*sizey*sizeof(double);
    mat=malloc(size);
    if (!mat) {
	perror("malloc");
	exit(1);
    }
    srand((unsigned)time(NULL));
    for (int i=0; i<sizex; i++) {
	for (int j=0; j<sizey; j++) {
	    double f=(double)rand()/RAND_MAX;
	    mat[i*sizey+j] = f;
	    if (f>amax) amax=f;
	    else if (f<amin) amin=f;
	}
    }
    for (int i=0; i<sizex; i++) {
	for (int j=0; j<sizey; j++) {
	    fprintf(stderr,"%f ",mat[i*sizey+j]);
	}
	fprintf(stderr,"\n");
    }
    printf("\n=====\nMin: %lf\nMax: %lf\n=====\n",amin,amax);




    FILE *fp=fopen("input.file","wb");
    if (!fp) {
	perror("fopen");
	exit(1);
    }
    if (fwrite(&sizex,1,sizeof(int),fp)<0) {
	perror("write");
	exit(1);
    }
    if (fwrite(&sizey,1,sizeof(int),fp)<0) {
	perror("write");
	exit(1);
    }
    if (fwrite(mat,1,size,fp)<0) {
	perror("write");
	exit(1);
    }
    fclose(fp);
    exit(0);
}



