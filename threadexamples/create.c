#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message_function (void *ptr);

main ()
{
    pthread_t thread1, thread2;
    char *message1 = "Thread 1";
    char *message2 = "Thread 2";
    pthread_t iret1, iret2;

    /* Create independent threads each of which will execute function */

    if (pthread_create
	(&thread1, NULL, print_message_function, (void *) message1)
	|| pthread_create (&thread2, NULL, print_message_function,
			   (void *) message2)) {
	perror ("pthread_create");
	exit (1);
    }

    /* Wait till threads are complete before main continues. Unless we  */
    /* wait we run the risk of executing an exit which will terminate   */
    /* the process and all threads before the threads have completed.   */

    pthread_join (thread1, (void *) &iret1);
    pthread_join (thread2, (void *) &iret2);

    printf ("Thread 1 returns: %u\n", iret1);
    printf ("Thread 2 returns: %u\n", iret2);
    exit (0);
}

void *print_message_function (void *ptr)
{
    char *message;
    pthread_t me;

    message = (char *) ptr;
    me = pthread_self ();
    printf ("%u:%s \n", me, message);
    pthread_exit ((void *) me);
}
