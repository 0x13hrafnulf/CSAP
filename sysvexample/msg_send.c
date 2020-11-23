#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>

#define MSGSZ     128*1024


/*
 * Declare the message structure.
 */

typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf;

main()
{
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf sbuf;
    size_t buf_length;
    struct msqid_ds buf;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 1234;

    (void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx, %#o)\n",
		   key, msgflg);

    if ((msqid = msgget(key, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }
    else 
	(void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);


    if (msgctl(msqid,IPC_STAT,&buf)<0) {
	perror("msgctl");
	exit(1);
    }
     
    /*
     * We'll send message type 1
     */
    sbuf.mtype = 1;
    (void) strcpy(sbuf.mtext, "Did you get this?");
    buf_length = strlen(sbuf.mtext) + 1 ;
//    buf_length = 8192;
    
    

    /*
     * Send a message.
     */
    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
	printf ("%d, %d, %s, %d\n", msqid, sbuf.mtype, sbuf.mtext, buf_length);
        perror("msgsnd");
        exit(1);
    }

    else {
	printf("Message: \"%s\" Sent\n", sbuf.mtext);
    }

    exit(0);
}
