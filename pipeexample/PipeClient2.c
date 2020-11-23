#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include "fullduplex.h"

int main(int argc, char *argv[])
{
    int wrfd, rdfd, numread;
    char rdbuf[MAX_BUF_SIZE];

    /* Check if an argument was specified. */

    if (argc != 2) {
        printf("Usage : %s <string to be sent to the server>n", argv[0]);
        exit (1);
    }

    /* Open the first named pipe for writing */
    wrfd = open(NP1, O_WRONLY);

    /* Open the second named pipe for reading */
    rdfd = open(NP2, O_RDONLY);

    /* Write to the pipe */
    write(wrfd, argv[1], strlen(argv[1])+1);

    /* Read from the pipe */
    numread = read(rdfd, rdbuf, MAX_BUF_SIZE);

    printf("Full Duplex Client : Read From the Pipe : %s\n", rdbuf);
}
