#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>


void handler(int sig)
{
    signal(SIGINT,handler); // Reinstall handler - Some Unixes require this
    printf("Ouch\n");
}

main()
{
    
    signal(SIGINT,handler);
    for (;;) ;
}
