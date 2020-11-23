#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define min(a,b) (a<b ? a : b)

main(int argc, char *argv[])
{
    int s,len,i;
    long l;
    struct sockaddr_in saddr;
    struct hostent *hst;
    char temp[80];


    if (argc<2) {
	printf("\nUsage:\n\t%s <string1> <string2> .. <stringn>\n",argv[0]);
	exit(1);
    }

    if ((s=socket(AF_INET,SOCK_STREAM,0))<0) {
	perror("socket");
	exit(1);
    }
    puts("socket done");

    if ((hst=gethostbyname("localhost"))==NULL) {
	perror("gethostbyname");
	exit(1);
    }
    printf("gethostbyname: %u %d\n",*(int*)hst->h_addr,hst->h_length);

    bcopy(hst->h_addr,&saddr.sin_addr,hst->h_length);
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(16000);

    if (connect(s,(struct sockaddr *)&saddr,sizeof(saddr))<0){
	perror("connect");
	exit(1);
    }
    puts("connect done");
    
    l=htonl(argc-1);
    if (write(s,&l,sizeof(long))<0) {
	perror("write");
	exit(1);
    }
    
    for (i=1; i<argc; i++) {
	long len;
	strncpy(temp,argv[i],min(sizeof(temp),strlen(argv[i])));
	temp[79]='\0';
	len=htonl(strlen(temp)+1);
    	if (write (s,&len,sizeof(len))<0 || write(s,temp,strlen(temp)+1)<0) {
	    perror("write");
	    exit(1);
    	}
    }
    printf("Data sent.\n");

    if (read(s,&l,sizeof(long))<0) {
	perror("read");
	exit(1);
    }
    
    if (ntohl(l) != argc-1 ) {
    	printf("Count mismatch!\n");
	exit(1);
    }
    printf("Count ok\n");

    for (i=1; i<argc; i++) {
	long len;
    	if (read(s,&len,sizeof(len))<0 || read(s,temp,(int)(ntohl(len)))<0) {
	    perror("write");
	    exit(1);
    	}
	printf("%d: %s\n",i,temp);
    }

    close(s);
}
