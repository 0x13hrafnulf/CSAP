#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

void dowork(int c)
{
    int ops[3];
    if (read (c, ops, sizeof (ops)) < 0) {
	perror ("read");
	exit (1);
    }

    ops[2] = ops[0] + ops[1];

    printf ("%d + %d = %d\n", ops[0], ops[1], ops[2]);

    if (write (c, ops, sizeof (ops)) < 0) {
	perror ("read");
	exit (1);
    }

    shutdown (c, 2);
    close (c);	    
}

main()
{
    int s,c,len;
    struct sockaddr_in saddr;
    int ops[3];
    int addr;
    int running;
    int maxspawn;

    if ((s=socket(AF_INET,SOCK_STREAM,0))<0) {
	perror("socket");
	exit(1);
    }
    puts("socket done");

    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(16000);
    saddr.sin_addr.s_addr=INADDR_ANY;

    if (bind(s,(struct sockaddr *)&saddr,sizeof(saddr))){
	perror("bind");
	exit(1);
    }
    puts("bind done");

    if (listen(s,5)) {
	perror("listen");
	exit(1);
    }
    puts("listen done");
    
    running=0;
    maxspawn=0;
    
    while (1) {
	len = sizeof (saddr);
	if ((c = accept (s, (struct sockaddr *) &saddr, &len)) < 0) {
	    perror ("accept");
	    exit (1);
	}
	addr = *(int *) &saddr.sin_addr;
	printf ("accept - connection from %d.%d.%d.%d\n",
		(addr & 0xFF), (addr & 0xFF00) >> 8, (addr & 0xFF0000) >> 16,
		(addr & 0xFF000000) >> 24);
	switch (fork()) {
	    case 0:
		close(s);
		dowork(c);
	    	exit(0);
	    case -1:
	    	perror("fork");
	    	break;
	    default:
	    	close (c);	    
	    	running +=1;
		if (running > maxspawn) {
		    maxspawn=running;
		    printf ("== Max:%d\n",maxspawn);
		}
		while (waitpid(-1,NULL, WNOHANG)>0) running -=1;
	    	break;
	}

    }
    close(s);

}
