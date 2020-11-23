#include <unistd.h>
#include <stdio.h>

#define VERYLONGMSG  327680
#define MSGSIZE  1024

int main(int argc, char *argv[])
{  
    char inbuf[MSGSIZE];
    char msg[VERYLONGMSG];

    int p[2],j,n,bytes;

    bzero(msg,VERYLONGMSG);
    for (j=0; j<VERYLONGMSG-1; j++) {
	msg[j]='a'+j%26;
    }
    
    if (argc!=2 || (bytes=atoi(argv[1]))>VERYLONGMSG || bytes<0) {
	fprintf(stderr,"Usage\n\t%s <nbytes>\n",argv[0]);
	exit(1);
    }


    /* open pipe */
    if(pipe(p)) {    
	perror("pipe");
        exit(1);
    }

    if ((j=fork())<0) {
	perror("fork");
	exit(1);
    }
    else if (j) {
    /* write down pipe */
	j=0;
	while (j<bytes) {
	    if ((n=write(p[1], msg, bytes))<0) {
		perror("write");
		exit(1);
	    }
	    printf("Wrote %d bytes.\n",n);
	    j +=n;
	}
    }
    else {
    /* read pipe */
	j=0;
	sleep(5);
	while (j<bytes) {
	    if ((n=read(p[0], inbuf, MSGSIZE))<0) {
		perror("read");;
	    }
	    printf("Read %d bytes.\n",n);
	    j +=n;
	}
    }

    exit(0);
}

