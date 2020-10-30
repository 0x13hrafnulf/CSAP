#include <stdio.h>
#include <stdlib.h>

int *add(int m, int n)
{
    static int sum=0;
    sum=m+n;
    return (&sum);
}

void main()
{
  int *p=add(10,15);
  printf("%p:%d\n",p,*p);
}


