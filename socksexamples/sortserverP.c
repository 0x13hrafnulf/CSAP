#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define min(a,b) (a<b ? a : b)

void bubble(char *ptr[])
{
    int sorted,i;
#if defined(DEBUG)
    for (i=0; ptr[i]; i++) printf("<< %d:%s\n",i,ptr[i]);
#endif
    
    do {
	sorted=1;
	i=0;
	do {
#if defined(DEBUG)
	    printf("%s %s %d\n",ptr[i],ptr[i+1],strcmp(ptr[i],ptr[i+1]));
#endif
    	    if (strcmp(ptr[i],ptr[i+1])>0) {
		char *temp;
		temp=ptr[i];
		ptr[i]=ptr[i+1];
		ptr[i+1]=temp;
		sorted=0;
	    }
	    i +=1;
	} while (ptr[i+1]);
    } while (!sorted) ;

#if defined(DEBUG)
    for (i=0; ptr[i]; i++) printf(">> %d:%s\n",i,ptr[i]);
#endif

    return;
}

void dowork(int c)
{
    char temp[80];
    int i,n;
    long l;
    char *ptr[1024];
    
    if (read (c, &l, sizeof (l)) < 0) {
	perror ("read");
	exit (1);
    }
    
    n=ntohl(l);
    printf("%ld items to be sorted.\n",n);
    i=0;
    while (i<min(n,sizeof(ptr)/sizeof(char *))) {
	long len;
	if (read(c,&len,sizeof(len))<0 || read (c, temp, (int)(ntohl(len))) < 0) {
	    perror ("read");
	    exit (1);
	}
    	ptr[i]=strdup(temp);
#if defined(DEBUG)
	printf("%d: %s\n",i,ptr[i]);
#endif
	i +=1;
    }
    ptr[i]=NULL;
    bubble(ptr);

    l=htonl(i);
    if (write (c, &l, sizeof(l)) < 0) {
	perror ("write");
	exit (1);
    }

    i=0;
    while (ptr[i]) {
	long len=htonl(strlen(ptr[i])+1);
	if (write(c,&len,sizeof(len))<0 || write (c, ptr[i],strlen(ptr[i])+1) < 0) {
	    perror ("write");
	    exit (1);
	}
	i+=1;
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
