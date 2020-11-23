#include <stdio.h> 
#include <string.h> 
#include <sys/wait.h> 

#define SMALLSIZE 8
 
int main(int argc, char *argv[]) 
{ 
    int p[2]; 
    int i, pid, status; 
    char buffer[SMALLSIZE];
    char *string1="Hello There";
    char *string2="- Waiting -";

    pipe(p); /* create the pipe   */ 
    if ((pid = fork()) == 0) {           
	/* in child - close write end of pipe */ 
	close(p[1]);           
	/* Read data from pipe into buffer */
	while ((i = read(p[0], buffer, sizeof(buffer)-1)) != 0) { 
	    buffer[i] = '\0'; 
	    printf("%d chars :%s: received by child\n", i, buffer); 
	} 
	
	close(p[0]);
	exit(0);
    } 
    /* in parent - close read end (parent writes p[1])    */ 
    close(p[0]);

    write(p[1],string1,strlen(string1)); 
    write(p[1],string2,strlen(string2)); 


    close(p[1]);                   
    /* waiting for pid       */ 
    wait(&status);    
    if (status == 0) {
	printf("child finished\n"); 
    }
    else {
	printf("child failed\n"); 
    }
    exit(0); 
} 

