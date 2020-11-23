#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define MSGSIZE 64

char *msg1 = "hello #1";
char *msg2 = "hello #2";
char *msg3 = "hello #3";

main()
{  
    char inbuf[MSGSIZE];

    int p[2], j, i;
    pid_t pid;

/* create pipe */
    if(pipe(p) == -1) {    
	perror("pipe call error");
	exit(1);
    }

    switch(pid = fork())
    {
    case -1: 
	perror("fork");
	exit(2);

    case 0:  
	/* child - write down pipe */
	/* first close the read end of the pipe */
	close(p[0]);  
	/* dup2 -> duplicates p[1] to 1 (stdout) */
	if(dup2(p[1],1)==-1) { 
	    perror( "dup2 failed" );
	    exit(1);
	}
	close(p[1]);
	printf("Child says:%s\n", msg1);
	printf("Child says:%s\n", msg2);
	printf("Child says:%s\n", msg3);
	break;

    default:   
	/* parent reads pipe */
	/* first close the write end of the pipe */
	close(p[1]);  
	/* dup2 -> duplicates p[0] to 0 (stdin) */
	if(dup2(p[0],0)==-1) { 
	    perror( "dup2 failed" );
	    exit(1);
	}
//	while(i=read(p[0], inbuf, sizeof(inbuf)-1)) {
	while(i=read(0, inbuf, sizeof(inbuf)-1)) {
	    inbuf[i]='\0';
	    printf("Parent: <%s>\n", inbuf);
	}
	wait(NULL);
	close(p[0]);
    }

    exit(0);
}

