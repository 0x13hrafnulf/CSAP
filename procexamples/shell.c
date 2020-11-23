#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE 1024
#define MAXARGS 64

int main(int argc, char *argv[])
{
    char buf[SIZE];
    char *args[MAXARGS];

    for (;;) {
        /*
         * Prompt for and read a command.
         */
        printf("Command: ");

        if (fgets(buf,sizeof(buf),stdin) == NULL) {
            printf("\n");
            exit(0);
        }

        /*
         * Split the string into arguments.
         */
        parse(buf, args);

        /*
         * Execute the command.
         */
        execute(args);
    }
}

/*
 * parse--split the command in buf into
 *         individual arguments.
 */
int parse(char *buf, char **args)
{
    int nargs=0;

    if (buf[strlen(buf)-1]=='\n') {
	buf[strlen(buf)-1]='\0';
    }

    while (*buf != '\0') {
        /*
         * Strip whitespace.  Use nulls, so
         * that the previous argument is terminated
         * automatically.
         */
        while ((*buf == ' ') || (*buf == '\t')) {
            *buf++ = '\0';
	}

	if (nargs>=MAXARGS) {
	    fprintf(stderr,"Too many arguments - ignoring rest of line\n");
	    *args = NULL;
	    return(1);
	}
        /*
         * Save the argument.
         */
        *args++ = buf;
	nargs +=1;

        /*
         * Skip over the argument.
         */
        while ((*buf != '\0') && (*buf != ' ') && (*buf != '\t')) {
            buf++;
	}
    }

    *args = NULL;
}

/*
 * execute--spawn a child process and execute
 *           the program.
 */
int execute(char **args)
{
    int pid, status;

    /*
     * Create a child process.
     */
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    /*
     * The child executes the code inside the if.
     */
    if (pid == 0) {
        execvp(*args, args);
        perror(*args);
        exit(1);
    }

    /*
     * The parent executes the wait.
     */
    while (wait(&status) != pid) sleep(1);
}
