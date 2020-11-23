#include "raw.c"

void main(int c, char **a)
{
    unsigned char message[128];
    int sd,ret,ncar=128;
    ret,ncar =128;
    unsigned char *eth;
    unsigned char abuf[6];
    unsigned char packet[1000];
    unsigned int i=0;
    int type=0,fn=0;
    char v=0;
    if(c<3) {
        printf("Usage : %s <interface> <frame_number> [d,p] \n",a[0]);
        exit(1);
    }
    sd=create_rawsocket(ETH_P_ALL);
    ret= bind_rawsocket(a[1],sd,ETH_P_ALL);
    fn=atoi(a[2]);
    if(c >3) v=*a[3];
    while(i <fn) {
        recv_rawpacket(sd,packet,ETHER_HDR_LEN+ncar);
        type= type_ethhdr(packet);
        if(v =='d') hex_dump(packet,64);
        switch (type) {
        case 1 :
            printf("IP protocol protocol \n");
            break;
        case 2 :
            printf("ARP protocol protocol \n");
            break;
        case 3 :
            printf("RARP protocol protocol \n");
            break;
        default:
            printf("other protocol \n");
            break;
        }
        i++;
        if(v =='p') print_ethhdr(packet);
    }
}
