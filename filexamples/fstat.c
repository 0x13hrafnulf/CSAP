#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

char *ptime(time_t t)
{
  struct tm *ts;
  static char string[30];
  ts=localtime(&t);
  strftime(string,sizeof(string),"%e %h -  %H:%M", ts);
  return(string);
}


int main(int argc, char *argv[])
{
    int fd;
    char *name;
    const int mode=O_RDONLY; 
    struct stat info;

    
    if (argc<2) {
	fprintf(stderr,"Usage\n\t%s <filein>",argv[0]);
	exit(1);
    }
    name=argv[1];

    if ((fd=open(name,mode))<0) {
	fprintf(stderr,"%s:",name);
	perror("open");
	exit(1);
    }

    if (fstat(fd,&info)) {
    	perror("stat");
    	exit(2);
    }

    printf("File Info:\n");

    printf("\tdevice:\t%x\n",info.st_dev);  
    printf("\tinode:\t%x\n",info.st_ino);  
    printf("\tprotection:\t%o\n",info.st_mode);  
    printf("\tnumber of hard links:\t%u\n",info.st_nlink);  
    printf("\tuser ID of owner:\t%u\n",info.st_uid);  
    printf("\tgroup ID of owner:\t%u\n",info.st_gid);  
    printf("\tdevice type (if inode device):%x\n",info.st_rdev);  
    printf("\ttotal size, in bytes:\t%u\n",info.st_size);  
    printf("\tblocksize for filesystem I/O:\t%u\n",info.st_blksize);  
    printf("\tnumber of blocks allocated:\t%u\n",info.st_blocks);  
    printf("\ttime of last access:\t%s\n",ptime(info.st_atime));  
    printf("\ttime of last modification:\t%s\n",ptime(info.st_mtime));  
    printf("\ttime of last status change\t%s\n",ptime(info.st_ctime));  

    if (close(fd)) perror("close");
}
