#include "raw.c"

#define SRC_ETHER_ADDR "aa:aa:aa:aa:aa:aa"
#define DST_ETHER_ADDR "ff:ff:ff:ff:ff:ff"
#define ETHER_TYPE 0x8000

void main(int c, char **a)
{
    unsigned char message[128];
    int sd,ret,fn,ncar,i=0;
    unsigned char *eth;
    unsigned char abuf[6];
    unsigned char packet[1000];
    unsigned short type=htons(ETHER_TYPE);
    type= htons(ETHER_TYPE);
    if(c ==1) {
        printf("Usage : %s [eth0, wlan0] frame_number \n ",a[0]);
        exit(1);
    }
    fn=atoi(a[2]);
    printf("Give the data message to send in the frame: \n");
    ncar = read(0,message,128);
    sd = create_rawsocket(ETH_P_ALL);
    ret= bind_rawsocket(a[1],sd,ETH_P_ALL);
    eth = (unsigned char *) create_eth(SRC_ETHER_ADDR,DST_ETHER_ADDR,ETHER_TYPE);
    create_eth(SRC_ETHER_ADDR,DST_ETHER_ADDR,ETHER_TYPE );
    memcpy(packet,eth,ETHER_HDR_LEN );
    memcpy(packet+ETHER_HDR_LEN,message,ncar);
    while(i <fn) {
        send_rawpacket(sd,packet,ETHER_HDR_LEN+ncar);
        sleep(1);
        i++;
    }
}
