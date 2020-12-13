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
void parse(int in_fd);
void read_dir(char *dirname, int *lines, int *words, int *chars);
void execute(char *name, int *lines, int *words, int *chars);

int main(int argc, char *argv[]) 
{
    int fd;
    char *name;


    if (argc < 2) {
        fprintf(stderr, "Usage: ./<program_name> <input_file>\n");
        exit(1);
    }

    fd = open(argv[1],O_RDONLY);
    if (fd < 0) {
	    fprintf(stderr,"%s:", fd);
	    perror("open");
	    exit(1);
	}

    parse(fd);  

    close(fd);
    return 0;
}


void parse(int in_fd)
{

    char buffer[SIZE];
    off_t offset = 0;
    ssize_t len = 0;

    int n_lines = 0;
    int n_words = 0;
    int n_chars = 0;

    while ((len = readline (in_fd, buffer, SIZE, &offset)) != -1)
    {       
        read_dir(buffer, &n_lines, &n_words, &n_chars);
    } 

    printf("%d %d %d\n", n_lines, n_words, n_chars);

    return 0;

}

void read_dir(char *dirname, int *lines, int *words, int *chars)
{
    DIR* dirh;
    struct dirent *dirp;
    struct stat info;
    char pathname[SIZE];
    int fd;



    //printf ("Directory: %s\n", dirname);

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

        if (S_ISREG(info.st_mode))
        {
            //printf("%s is a file\n",pathname);

            //printf("%.f difference\n", dif);
            
            pathname[strlen(pathname)] = '\0';


            execute(pathname, lines, words, chars);

        }

        if (S_ISDIR(info.st_mode)) 
        {
            //printf("%s is a directory\n",pathname);
            read_dir(pathname, lines, words, chars);
        }


        close(fd);
    }
}
void execute(char *name, int *lines, int *words, int *chars)
{
    int pid, status;
    char read_buffer[SIZE/2];
    int c;
    char *parsed_string;

    int channel[2]; 
    if(pipe(channel) == -1) {    
        perror("pipe call error");
        exit(1);
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) 
    {
        close(1); 	    
        if (dup2(channel[1], 1) < 0) {
            perror("dup2");
            exit(1);
        }   

        execl("/bin/wc", "wc", name, (char*)NULL);
        exit(1);
    }
    else
    {
        close(channel[1]);
        int i = 0;
        while ((c = read(channel[0], read_buffer, sizeof(read_buffer)-1)) != 0) { 
            read_buffer[c] = '\0'; 

            parsed_string = strtok(read_buffer, " ");
            
            while (parsed_string != NULL)
            {
                switch (i)
                {
                    case 0: 
                        *lines += atoi(parsed_string);
                        break;
                    case 1: 
                        *words += atoi(parsed_string);
                        break;  
                    case 2: 
                        *chars += atoi(parsed_string);
                        break;     
                }

                parsed_string = strtok (NULL, " ");
                ++i;
            }

            //printf("%s: %d bytes received by child\n", read_buffer, c);

        }
        close(channel[0]);

        int status;
        //printf("Waiting for ANY child\n");
        if ((pid = waitpid(-1,&status,0)) < 0) {
            perror("waitpid");
        }
        printf ("Child:%d returned %d\n", pid, WEXITSTATUS(status));

    }

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

