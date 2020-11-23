#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include "fullduplex.h"

int main(int argc, char *argv[])
{
    int rdfd, wrfd, ret_val, count, numread;
    char buf[MAX_BUF_SIZE];

    /* Create the first named - pipe */
    ret_val = mkfifo(NP1, 0666);

    if ((ret_val == -1) && (errno != EEXIST)) {
        perror("Error creating the named pipe");
        exit (1);
    }

    ret_val = mkfifo(NP2, 0666);

    if ((ret_val == -1) && (errno != EEXIST)) {
        perror("Error creating the named pipe");
        exit (1);
    }

    /* Open the first named pipe for reading */
    rdfd = open(NP1, O_RDONLY);

    /* Open the second named pipe for writing */
    wrfd = open(NP2, O_WRONLY);

    /* Read from the first pipe */
    numread = read(rdfd, buf, MAX_BUF_SIZE);

    printf("Full Duplex Server : Read From the  pipe : %s\n", buf);

    /* Convert to the string to upper case */
    count = 0;
    while (count < numread) {
        buf[count] = toupper(buf[count]);
        count++;
    }

    /* 
     * Write the converted string back to the second 
     * pipe 
     */    
    write(wrfd, buf, strlen(buf)+1);
}
