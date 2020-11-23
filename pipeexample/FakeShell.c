#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#define PROMPT ">>"
#define LINELEN 2048
#define MAXARGS 10
#define DELIM " \n&"

void myexit(int t)
{
    printf("Child has exited");
    exit(1);
}

int main()
{
    int pid;
    int apipe[2];
    int len;
    int i;
    int background;
    char line[LINELEN];
    int fin,fout;
    char *program;
    char *args[MAXARGS];
    char *p;
    char buffer[LINELEN];

    fin=0; 	// stdin
    fout=1; 	// stdout

    while (1) {
	bzero(line,sizeof(line)); // Cleanup input buffer
	
	// Get Input String
	if (write(fout,PROMPT,strlen(PROMPT))<0) { 
	    perror("write");
	    exit(1);
	}
	if (read(fin,line,LINELEN)<0) {
	    perror("read");
	    exit(1);
	}
	len=strlen(line);
	if (len<=1) continue;


	// Background? (& as suffix)
	if (strrchr(line,'&'))  background=1;
	else background=0;

	// Get program name (parsing delimiters)
	program=strtok(line,DELIM);
	if (!program) continue;


	args[0]=program;

	// Get Arguments
	i=1;
	do {
	    p=strtok(NULL,DELIM);
	    if (p) args[i++]=p;
	} while (p);
	args[i]=NULL;

	// Create pipes
	if (pipe(apipe)) {
	    perror("pipe");
	    exit(1);
	}

	switch (pid=fork())
	{
	case 0: // Child 

	    // Connect pipe to stdout
	    close(1); 	    
	    if (dup2(apipe[1],1)<0) {
		perror("dup2");
		exit(1);
	    }
	    
	    // Exec program
	    execvp(program,args);

	    // ?? Exec failed
	    fprintf(stderr,"%s:",program);
	    perror("execvp");
	    exit(1);

	case -1:
	    // ?? Fork failed
	    perror("fork");
	    exit(1);

	default:
	    // Close write end of pipe
	    close(apipe[1]);
	    // Fork the output "reader" 
	    if (!(pid=fork())) {
		// Get output from spawned command
		i=read(apipe[0],buffer,LINELEN-2);
		if (i<0) {
		    perror("read");
		}
		else {
		    // Write output to console
		    buffer[i]='\n';
		    buffer[i+1]='\0';
		    write(1,buffer,i+1);
		}
		exit(0);
	    }
	    else {
		// Close read end
		close(apipe[0]);

		// Wait to termination (unless backgrounded)
		if (!background) waitpid(pid,NULL,0);
		
	    }
	    // Reap defunct processes
	    while(waitpid(-1,NULL,WNOHANG)>0);
	    break;
	}
    }
}
