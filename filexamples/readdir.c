#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFSIZE 255

int main (int argc, char *argv[])
{ 
    DIR *dirh;
    struct dirent *dirp;
    struct stat statbuf;
    char pathname[BUFSIZE];
    char linkname[BUFSIZE];
    char *dirname;
    int fd;

    if (argc!=2) {
	fprintf(stderr,"Usage:\n\t%s <directory>\n",argv[0]);
	exit(1);
    }

    dirname=argv[1];
    if ((dirh = opendir(dirname))==NULL) {
	perror("opendir");
	exit(1);
    }
 
    for (dirp = readdir(dirh); dirp != NULL; dirp = readdir(dirh)) {

	if (strcmp(".",dirp->d_name) == 0 || 
	    strcmp("..",dirp->d_name) == 0 || 
	    strcmp("lost+found",dirp->d_name) == 0) {
	    continue;
	}

	sprintf(pathname,"%s/%s",dirname,dirp->d_name);

	if ((fd=open(pathname,O_RDONLY))<0) {
	    fprintf(stderr,"%s:",pathname);
	    perror("open");
	    continue;
	}

	if (fstat(fd,&statbuf) == -1) {
	    perror("stat");
	    close(fd);
	    continue;
	}

	if (S_ISREG(statbuf.st_mode)) {
	    printf("%s is a file,",pathname);
	}

	if (S_ISDIR(statbuf.st_mode)) {
	    printf("%s is a directory,",pathname);
	}

	if (S_ISLNK(statbuf.st_mode)) {
	    bzero(linkname,BUFSIZE);   
	    readlink(pathname,linkname,BUFSIZE);
	    printf("%s is a link to %s,",pathname,linkname);
	}

	printf("mode:%o\n\n",statbuf.st_mode & 07777);
	close(fd);
    }

    closedir(dirh);
}
