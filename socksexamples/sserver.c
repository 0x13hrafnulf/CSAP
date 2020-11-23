#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

main()
{
    int s,c,len;
    struct sockaddr_in saddr;
    int ops[3];
    int addr;

    // Create the socket
    if ((s=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
	perror("socket");
	exit(1);
    }
    puts("socket done");

    saddr.sin_family=PF_INET;
    saddr.sin_port=htons(16000);
    saddr.sin_addr.s_addr=INADDR_ANY;

    // Assign address (16000), protocol and peer address
    if (bind(s,(struct sockaddr *)&saddr,sizeof(saddr))){
	perror("bind");
	exit(1);
    }
    puts("bind done");

    // Tell the kernel the socket is ready to accept connections
    // No pending connections, since we'll terminate immediately
    if (listen(s,0)) {
	perror("listen");
	exit(1);
    }
    puts("listen done");

    len=sizeof(saddr);
    // Wait for connections - <<<C is the NEW socket>>>
    if ((c=accept(s,(struct sockaddr *)&saddr,&len))<0) {
	perror("accept");
	exit(1);
    }
    addr=*(int *)&saddr.sin_addr;
    printf("accept - connection from %d.%d.%d.%d\n",
	   (addr&0xFF),(addr&0xFF00)>>8,(addr&0xFF0000)>>16,
	   (addr&0xFF000000)>>24);
    
    // Read (or recv) from socket
    if (read(c,ops,sizeof(ops))<0) {
	perror("read");
	exit(1);
    }

    ops[2]=ops[0]+ops[1];

    printf("%d + %d = %d\n",ops[0],ops[1],ops[2]);

    // Write (or send) on socket
    if (write(c,ops,sizeof(ops))<0) {
	perror("read");
	exit(1);
    }
    
    // Shutdown socket -- a good habit
    shutdown(c,SHUT_RDWR);
    // Close BOTH sockets
    close(c);
    close(s);
}
