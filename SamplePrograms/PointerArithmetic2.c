#include<stdio.h>

int main()
{
    int i[10];
    int *ip=i;

    printf("Value of ip = %p\n", ip);
    printf("Address of i = %p\n", i);

    printf("================\n", ip);
    printf("Value of ip + 1 = %p\n", ++ip) ;
    printf("Address of i[1] = %p\n", &i[1]);
    printf("Address of i[1] = %p\n", i+1);

    return 0;
}
