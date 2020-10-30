#include <stdio.h>
#include <stdlib.h>

void main(int argc, char *argv[])
{
  printf("Number of args on command line:%d\n",argc);
  for (int i=0; i<argc; i++) {
    printf("Arg#%d:<%s>\n",i,argv[i]);
 }
}
