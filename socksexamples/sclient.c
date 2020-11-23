#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

main(int argc, char *argv[])
{
    int s,len;
    struct sockaddr_in saddr;
    struct hostent *hst;
    int ops[3];


    if (argc!=3) {
	printf("\nUsage:\n\t%s <op1> <op2>\n",argv[0]);
	exit(1);
    }

    // Create the socket
    if ((s=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
	perror("socket");
	exit(1);
    }
    puts("socket done");

    // Determine host address by its network name
    if ((hst=gethostbyname("localhost"))==NULL) {
	perror("gethostbyname");
	exit(1);
    }
    printf("gethostbyname: %u %d\n",*(int*)hst->h_addr,hst->h_length);

    // Fill structure
    bcopy(hst->h_addr,&saddr.sin_addr,hst->h_length);
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(16000);

    // Connect to other socket
    if (connect(s,(struct sockaddr *)&saddr,sizeof(saddr))<0){
	perror("connect");
	exit(1);
    }
    puts("connect done");

    ops[0]=atoi(argv[1]);
    ops[1]=atoi(argv[2]);
    
    // Write (or send) to socket
    if (write(s,ops,sizeof(ops))<0) {
	perror("write");
	exit(1);
    }

    // Read (or recv) from socket
    if (read(s,ops,sizeof(ops))<0) {
	perror("read");
	exit(1);
    }

    printf("%d + %d = %d\n",ops[0],ops[1],ops[2]);

    // Close the client socket
    close(s);
}
