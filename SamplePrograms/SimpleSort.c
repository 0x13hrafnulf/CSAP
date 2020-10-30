#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NLINES	    5
#define MAXLINELEN  80

void print(char (*p)[MAXLINELEN], int n)
{
  int i=0;
  for (i=0; i<n; i++) {
    printf("<%s>\n",p[i]);
  }
}

int getl(char *p, int maxl)
{
  int ch=0;
  int i=0;

  while (1) {
    if ((ch=getchar())<0) {
      return(ch);
    }

    if (ch!='\n') {
      p[i]=ch;
      p[++i]='\0';

      if (i>=maxl-1) {
	return(0);
      }
    }
    else {
      return(0);
    }
  }
}

void sort(char (*p)[MAXLINELEN], int n)
{
  for (int i=0; i<n-1; i++) {
    for (int j=i+1; j<n; j++) {
      if (strcmp(p[i],p[j])>0) {
	char tmp[MAXLINELEN];
	strcpy(tmp,p[i]);
	strcpy(p[i],p[j]);
	strcpy(p[j],tmp);
      }
    }
  }
}


void main()
{
  int i=0;
  char array[NLINES][MAXLINELEN]={'\0'};
  char line[MAXLINELEN];

  for (i=0; i<NLINES; i++) {
    if (getl(array[i],MAXLINELEN)<0) {
      exit(-1);
    }
  }
  printf("===== Before ====\n");
  print(array,NLINES);
  sort(array,NLINES);
  printf("===== After ====\n");
  print(array,NLINES);
}
