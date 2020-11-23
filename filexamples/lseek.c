#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BLKSIZE 1024

int main(int argc,char *argv[])
{
    char *name,*string;
    char buffer[BLKSIZE];
    int fd,block;
    off_t offset;
    struct stat info;

    if (argc<3) {
	fprintf(stderr,"Usage\n\t%s <file> <block> [<string>]\n",argv[0]);
	exit(1);
    }

    name=argv[1];

    block=atoi(argv[2]);
    if (block<0) {
	fprintf(stderr,"block#<0\n");
	exit(1);
    }

    if (argc>3) string=argv[3];
    else string=NULL;

    if ((fd=open(name,O_RDWR|O_CREAT,S_IRWXU))<0) {
	fprintf(stderr,"%s:",name);
	perror("open");
	exit(2);
    }

    offset=block*BLKSIZE;

    if (fstat(fd,&info)) {
    	perror("stat");
    	exit(3);
    }

    if (info.st_size<offset && !string) {
	fprintf(stderr,"Read beyond EOF\n");
	exit(4);
    }
	

    if (lseek(fd,offset,SEEK_SET)<0) {
	perror("lseek");
	exit(5);
    }

    if (!string) {
	if (read(fd,buffer,sizeof(buffer))<0) {
	    perror("read");
	    exit(6);
	}
	printf("Data:%s\n",buffer);
    }
    else {
	if (write(fd,string,strlen(string)+1)<0) {
	    perror("write");
	    exit(7);
	}
    }
    close(fd);
}
