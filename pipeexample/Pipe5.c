#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Define command strings */
#define COMMAND1 "ls"
#define COMMAND2 "grep"
#define ARG1 "Pipe"

int main()
{
    int Pipe[2];
    int childPID;   

    /* create pipe */
    if (pipe(Pipe)) {
	perror("pipe");
	exit(1);
    }

    /* create child */
    if((childPID=fork())<0) {
        perror( "Fork Error" );
        exit(-1);
    }

    if(!childPID) {
	/* child */
        /* Force our stdin to be the read side of the pipe we made */
	close(0);
	close(Pipe[1]);
        if (dup2(Pipe[0],0)<0) {
	    perror("dup2");
	    exit(1);
	}
        printf("\n");

        /* Execute our command */
        execlp(COMMAND2,COMMAND2,ARG1,NULL);
        perror("Execl error");
        exit(-1);
    }

    /* 
     * Force our stdout to be the write end of the Pipe we formed. 
     */ 
    close(Pipe[0]);
    close(1);
    if (dup2(Pipe[1],1)!=1) {
	perror("dup2");
	exit(1);
    }

    
    /* Execute our command */
    execlp(COMMAND1,COMMAND1,NULL);
    perror("EXEC Error");
    return(0);
}
