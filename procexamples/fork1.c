#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 1024

int main(int argc, char *argv[])
{
    int fd,i;
    char string[SIZE];

    if (argc<2) {
	fprintf(stderr,"Usage:\n\t%s <filename>\n",argv[0]);
	exit(1);
    }

    fd=open(argv[1],O_WRONLY|O_CREAT,S_IRWXU);

    if (fd<=0) {
	perror("open:");
	exit(1);
    }

    if (fork()<0) {
	perror("fork:");
	exit(1);
    }
    
    sprintf(string,"I'm process %d\n",getpid());
    for (i=0; i<10000; i++) {
	if (write(fd,string,strlen(string))<0) {
	    perror("write:");
	    close(fd);
	    exit(1);
	}
	usleep(1000);
    }
    close(fd);
}

