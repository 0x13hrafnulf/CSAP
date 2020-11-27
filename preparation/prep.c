#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define SIZE 1024

ssize_t readline (int fd, char *buf, size_t sz, off_t *offset);
void print(int out_fd);
void parse(int in_fd, int out_fd);
void read_dir(char *dirname, pid_t pid, time_t time, int channel);



int main(int argc, char *argv[]) 
{
    int fd[2];
    char *name[2];


    if (argc != 3) {
        fprintf(stderr, "Usage: ./<program_name> <output_file> <input_file> <opitonal program to be executed>\n");
        exit(1);
    }

    fd[0] = open(argv[1],O_WRONLY|O_CREAT, S_IRWXU);
    if (fd[0]<0) {
	    fprintf(stderr,"%s:",fd[0]);
	    perror("open");
	    exit(1);
	}

    fd[1] = open(argv[2],O_RDONLY);
    if (fd[1]<0) {
	    fprintf(stderr,"%s:", fd[1]);
	    perror("open");
	    exit(1);
	}

    parse(fd[1], fd[0]);
    


    if(argc == 4)
    {


    }


    close(fd[0]);
    close(fd[1]);
    return 0;
}


ssize_t readline (int fd, char *buf, size_t sz, off_t *offset)
{

    ssize_t nchr = 0;
    ssize_t idx = 0;
    char *p = NULL;

    /* position fd & read line */
    if ((nchr = lseek (fd, *offset, SEEK_SET)) != -1)
        nchr = read (fd, buf, sz);

    if (nchr == -1) {   /* read error   */
        fprintf (stderr, "%s() error: read failure in '%d'.\n",
                __func__, fd);
        return nchr;
    }

    /* end of file - no chars read
    (not an error, but return -1 )*/
    if (nchr == 0) return -1;

    p = buf;    /* check each chacr */
    while (idx < nchr && *p != '\n') p++, idx++;
    *p = 0;

    if (idx == nchr) {  /* newline not found  */
        *offset += nchr;

        /* check file missing newline at end */
        return nchr < (ssize_t)sz ? nchr : 0;
    }

    *offset += idx + 1;

    return idx;
}

void parse(int in_fd, int out_fd)
{
    int n = 1;
    char buffer[SIZE];
    off_t offset = 0;
    ssize_t len = 0;
    size_t i = 0;
    time_t time;

    int c;
    

    pid_t *childs;
    pid_t child_pid;

    int channel[2]; 
    if(pipe(channel) == -1) {    
        perror("pipe call error");
        exit(1);
    }

    char read_buffer[SIZE/2];
    int k = 0;
    while ((len = readline (in_fd, buffer, SIZE, &offset)) != -1)
    {
        if(i == 0) 
        {
            n = atoi(buffer);
            if (!(childs=malloc(n*sizeof(pid_t)))) {
                perror("malloc");
                exit(1);
            }
            printf ("%d processes\n", n);
            ++i;
        }
        else if(i == 1)
        {
            struct tm tm;
            strptime(buffer, "%D/%M/%Y", &tm);
            time = mktime(&tm);
            ++i;
        }
        else {
            ++i;
            child_pid = fork();
            if (child_pid < 0) {
                perror("Fork");
                exit(1);
            }
            else if (child_pid == 0) 
            {
                int pid = getpid();
                printf ("PID: %d, Line[%2zu] : %s (%zd chars)\n", pid, i, buffer, len);
                close(channel[0]);
                read_dir(buffer, pid, time, channel[1]);
                close(channel[1]);
                exit(0);
            }
            else {
                childs[k] = child_pid;              
            } 

        }
    } 

    close(channel[1]);
    while ((c = read(channel[0], read_buffer, sizeof(read_buffer)-1)) != 0) { 
        read_buffer[c] = '\0'; 
        write(out_fd, read_buffer, c);
        printf("%s: %d bytes received by child\n", read_buffer, c); 
    }
    close(channel[0]);

    for(int j = 0; j < n; ++j)
    {
        int status;
        printf("Waiting for ANY child\n");
        if ((child_pid = waitpid(-1,&status,0)) < 0) {
            perror("waitpid");
        }
        printf ("Child:%d returned %d\n", child_pid, WEXITSTATUS(status));
    }


    free(childs);
    return 0;

}

void read_dir(char *dirname, pid_t pid, time_t time, int channel)
{
    DIR* dirh;
    struct dirent *dirp;
    struct stat info;
    char pathname[SIZE];
    int fd;

    printf ("PID: %d, Directory: %s\n", pid, dirname);

    if ((dirh = opendir(dirname)) == NULL) {
        perror("opendir");
        exit(1);
    }

    for (dirp = readdir(dirh); dirp != NULL; dirp = readdir(dirh)) 
    {

        if (strcmp(".",dirp->d_name) == 0 || 
            strcmp("..",dirp->d_name) == 0 || 
            strcmp("lost+found",dirp->d_name) == 0) {
            continue;
        }

        snprintf(pathname, sizeof(pathname) , "%s/%s", dirname, dirp->d_name);

        if ((fd = open(pathname,O_RDONLY)) < 0) {
            fprintf(stderr,"%s:",pathname);
            perror("open");
            continue;
        }

        if (fstat(fd, &info) == -1) 
        {
            perror("stat");
            close(fd);
            continue;
        }
        //printf("mode:%o\n",info.st_mode & 07777);

        if (S_ISREG(info.st_mode))
        {
            //printf("%s is a file\n",pathname);

            double dif = difftime(info.st_mtime, time);
            //printf("%.f difference\n", dif);
            printf("Child:%d writing %d bytes\n", pid, strlen(pathname));
            pathname[strlen(pathname)] = '\n';

            if(dif > 0)
            {
                int out = write(channel, pathname, strlen(pathname)); 
                if(out < strlen(pathname))
                {
                    if (out < 0) perror("Error write in child");
	                else printf("Child:%d wrote only %d out of %d bytes\n", pid, out, strlen(pathname));
                }
            }
        }

        if (S_ISDIR(info.st_mode)) 
        {
            //printf("%s is a directory\n",pathname);
            read_dir(pathname, pid, time, channel);
        }


        close(fd);
    }



}


