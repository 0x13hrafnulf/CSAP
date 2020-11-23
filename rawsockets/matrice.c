#include <stdio.h>
#include <stdlib.h>

void fill(int *p, int m, int n)
{
    int i,j;
    for (i=0; i<m; i++) {
        for (j=0; j<n; j++) {
            p[i*n+j]=rand();
        }
    }
}

void avg(double *avr, double *avc, int *p, int m, int n)
{
    int i,j;
    long sum;

    for (i=0; i<m; i++) {
        sum=0L;
        for (j=0; j<n; j++) {
            sum += p[i*n+j];
        }
        avr[i]=(double)sum/n;
    }

    for (j=0; j<n; j++) {
        sum=0L;
        for (i=0; i<m; i++) {
            sum += p[i*n+j];
        }
        avc[j]=(double)sum/m;
    }
}

void dump(int *p, int m, int n)
{
    int i,j;
    printf("\n\n>>Matrice\n");
    for (i=0; i<m; i++) {
        for (j=0; j<n; j++) {
            printf("%d ",p[i*n+j]);
        }
        printf("\n");
    }
}

void main()
{
    int m,n,i;
    int *mat=NULL;
    double *avr=NULL, *avc=NULL;

    scanf("%d %d",&m,&n);

    mat=malloc(m*n*sizeof(*mat));
    avr=malloc(m*sizeof(*avr));
    avc=malloc(n*sizeof(*avc));

    if (!mat||!avc||!avr) exit(1);

    fill(mat,m,n);
    dump(mat,m,n);

    avg(avr, avc, mat, m, n);

    printf("\n\n>>Media Righe:\n");
    for (i=0; i<m; i++) {
        printf("%f\n",avr[i]);
    }

    printf("\n\n>>Media Colonne:\n");
    for (i=0; i<n; i++) {
        printf("%f\n",avc[i]);
    }
}
