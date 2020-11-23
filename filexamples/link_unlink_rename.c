#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>

int statfile(int fd)
{
    struct stat info;

    if (fstat(fd,&info)) {
    	perror("stat");
    	return(-1);
    }
    return(info.st_nlink);
}

void printnlink(int fd)
{
    int nlink;

    if ((nlink=statfile(fd))<0) {
	exit(1);
    }
    printf("Number of hard links:\t%u\n",nlink);  
}

int main(int argc, char *argv[])
{
    int fd,nlink;
    char *name,*newname;
    struct stat info;

    
    if (argc<2) {
	fprintf(stderr,"Usage\n\t%s <file>\n",argv[0]);
	exit(1);
    }
    name=argv[1];

    if ((fd=open(name,O_RDONLY))<0) {
	fprintf(stderr,"%s:",name);
	perror("open");
	exit(1);
    }

    printnlink(fd);
    printf("Creating new (hard) link to file %s\n",name);
    if (!(newname=malloc(strlen(name)+3))) {
	perror("malloc");
	exit(1);
    }

    strcpy(newname,name);
    strcat(newname,"-1");

    if (link(name,newname)<0) {
	perror("link");
	exit(1);
    }

    printnlink(fd);

    printf("Removing (hard) link to file %s\n",name);
    unlink(name);
    printnlink(fd);

    if (close(fd)) perror("close");

    if (rename(newname,name)<0) {
	perror("rename");
	exit(1);
    }

}
