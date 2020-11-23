#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>


int main(int argc, char *argv[])
{
    int fd[2];
    char *name[2];
#if defined(CREATE)
    const int modes[]={O_RDONLY,O_WRONLY|O_CREAT};
    const int flags=S_IRWXU;
#else
    const int modes[]={O_RDONLY,O_WRONLY}; 
    const int flags=0;
#endif
    int i,o;
    char *buffer;
    struct stat info;
    int BUFSIZE;

    
    if (argc<3) {
	fprintf(stderr,"Usage\n\t%s <filein> <fileout>\n",argv[0]);
	exit(1);
    }
    for (i=0; i<2; i++) {
	name[i]=argv[i+1];
	if ((fd[i]=open(name[i],modes[i],flags))<0) {
	    fprintf(stderr,"%s:",name[i]);
	    perror("open");
	    exit(1);
	}
    }
    if (fstat(fd[0],&info)) {
    	perror("stat");
    	exit(2);
    }
    printf("File Size:%d bytes\n",info.st_size);

    if ((buffer=malloc(info.st_size))==NULL) {
	perror("malloc");
    }


    if ((i=read(fd[0], buffer,info.st_size))==info.st_size) {
	if ((o=write(fd[1],buffer,i))<i) {
	    if (o<0) perror("write");
	    else printf("Wrote only %d out of %d bytes\n",o,i);
	}
    }
    else {
	perror("read");
    }

    for (i=0; i<2;  i++) {
	if (close(i)) perror("close");
    }
}
