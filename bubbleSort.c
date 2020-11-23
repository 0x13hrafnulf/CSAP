#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print(char *p[], int n)
{
  int i=0;
  for (i=0; i<n; i++) {
    printf("<%s>\n",p[i]);
  }
}


void sort(char *p[], int n)
{
  int done=0;

  while (!done) {
    int item=n-1;
    done=1;

    while (item>0) {
      if (strcmp(p[item],p[item-1])<0) {
	char *temp=p[item];
	p[item]=p[item-1];
	p[item-1]=temp;
	done=0;
      }
      item -=1;
    }
  }
}


void main(int argc, char *argv[])
{
  FILE *fp=NULL;
  int nlines=10,i=0;
  char **array=NULL;
  char line[132];

  if (argc!=2 || !(fp=fopen(argv[1],"r"))) {
    exit(1);
  }
  
  array=malloc(nlines*sizeof(*array));
  if (!array) {
    perror("malloc");
    exit(1);
  }

  i=0;
  while (fgets(line,sizeof(line),fp)) {
    int l=strlen(line);

    array[i]=malloc(l);
    if (!array[i]) {
      perror("malloc");
      exit(1);
    }
    
    strncpy(array[i],line,l-1);
    array[i][l-1]='\0';

    i +=1;

    if (i>=nlines) {
      nlines=nlines*2;
      array=realloc(array,nlines*sizeof(*array));

      if (!array) {
	perror("malloc");
	exit(1);
      }
    }
  }
  print(array,i);
  sort(array,i);
  print(array,i);
}
