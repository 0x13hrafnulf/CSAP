#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>




void parse_mac(void *v, char *add)
{
    char *a=(char *)v;
    char *fmt="%hhx:%hhx:%hhx:%hhx:%hhx:%hhx";
    int rc = sscanf(add, fmt, a + 0, a + 1, a + 2, a + 3, a + 4, a + 5);
    if(rc != 6) fprintf(stderr, "invalid mac address format %s\n",add);
}


int create_rawsocket(int protocol_to_sniff )
{
    int rawsock ;
    if((rawsock = socket(PF_PACKET , SOCK_RAW,
                         htons(protocol_to_sniff )))== - 1) {
        perror("Error creating raw socket: ");
        exit( - 1);
    }
    return rawsock ;
}

int bind_rawsocket(char *device, int rawsock , int protocol)
{
    struct sockaddr_ll sll ;
    struct ifreq ifr ;
    bzero(&sll , sizeof(struct sockaddr_ll ));
    bzero(&ifr , sizeof(struct ifreq ));
    /* First Get the Interface Index */
    strncpy((char *) ifr.ifr_name , device, IFNAMSIZ);
    if((ioctl(rawsock , SIOCGIFINDEX , & ifr )) == - 1) {
        perror("Error getting Interface index !");
        exit( - 1);
    }
    /* Bind our raw socket to this interface */
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex ;
    sll.sll_protocol = htons(protocol );
    if((bind(rawsock , ( struct sockaddr *)& sll , sizeof(sll )))== - 1) {
        perror("Error binding raw socket to interface \n");
        exit( - 1);
    }
    return 1;
}

int send_rawpacket(int rawsock , unsigned char * pkt ,
                   int pkt_len )
{
    int sent= 0;
    if((sent = write (rawsock , pkt , pkt_len )) != pkt_len ) {
        printf("Could only send %d bytes of packet of length %d \n ", sent, pkt_len );
        return 0;
    }
    return 1;
}

int recv_rawpacket(int rawsock , unsigned char * pkt ,
                   int pkt_len )
{
    int recv = 0;
    recv = read (rawsock , pkt , pkt_len );
    return recv ;
}

unsigned char * create_eth(char * src_mac , char * dst_mac ,
                           unsigned short protocol)
{
    unsigned char * ethbuf ;
    unsigned char abuf[6];
    unsigned padding;
    unsigned short type= htons (protocol );
    ethbuf = (unsigned char *) malloc(14);
    parse_mac(abuf,dst_mac ); // from ff:ff:ff:ff:ff:ff form
    memcpy(ethbuf,abuf,6);
    parse_mac(abuf,src_mac );
    memcpy(ethbuf+6,abuf,6);
    memcpy(ethbuf+12,(unsigned char *)&type,2);
    return ethbuf ;
}

void print_ethhdr(unsigned char * eth_headstr )
{
    unsigned char * ethhead ;
    int j;
    ethhead = eth_headstr ;
    printf("Ethernet header\n destination address: ");
    for(j =0; j<6; j++) printf("%02x:",*( ethhead+j ));
    printf (" source address: ");
    for(j =6; j<12; j++) printf("%02x:",*( ethhead+j ));
    printf (" protocol number: ");
    for(j =12; j<14; j++) printf("%02x",*( ethhead+j ));
    printf(" \nend of Ethernet header \n");
}

int type_ethhdr(unsigned char * ethhead)
{
    if (*(ethhead+12)==8 && *(ethhead+13)==0) return 1; // IP
    if (*(ethhead+12)==8 && *(ethhead+13)==6) return 2; // ARP
    return 0;
}

void hex_dump(unsigned char *packet,int len)
{
    for (int i=0; i<len; i++) printf("%.2x ",packet[i]);
    printf("\n");
}

unsigned char * create_iphdr (
    unsigned char verlen ,
    unsigned char tos ,
    unsigned short totlen ,
    unsigned short id,
    unsigned short foffset ,
    unsigned char ttl ,
    unsigned char proto,
    unsigned short checksum,
    unsigned int sa ,
    unsigned int da )
{
    struct iphdr * ip_header ;
    /*
    unsigned char ip_version_and_header_length ;
    unsigned char ip_tos ; // type of service
    unsigned short ip_len ; // total lengthether_dest_addr
    unsigned short ip_id ; // identification number
    unsigned short ip_frag_offset ; // fragment offset and flags
    unsigned char ip_ttl ; // time to live
    unsigned char ip_type ; // protocol type
    unsigned short ip_checksum ; // checksum
    unsigned int ip_src_addr ; // source IP address
    unsigned int ip_dest_addr ; // destination IP address
    */
    ip_header = ( struct iphdr *) malloc(sizeof(struct iphdr ));
    ip_header -> tos = tos ;
    ip_header -> tot_len = totlen ;
    ip_header -> id = id;
    ip_header -> frag_off = foffset ;
    ip_header -> ttl = ttl ;
    ip_header -> protocol = proto;
    ip_header -> check = checksum;
    ip_header -> saddr = sa;
    ip_header -> daddr = da ;
    return ((unsigned char *) ip_header );
}


void print_iphdr(unsigned char * ip_headerstr )
{
    struct iphdr * ip_header ;
    unsigned char sa[4];
    unsigned char da[4];
    ip_header = ( struct iphdr *) malloc(sizeof(struct iphdr ));
    memcpy(ip_header,ip_headerstr,sizeof(struct iphdr ));
    memcpy(sa,(unsigned char *)& ip_header ->saddr,4);
    memcpy(da,(unsigned char *)& ip_header ->daddr,4);
    printf("Length of IP packet: %d\n",ntohs(ip_header -> tot_len ));
    printf("Identifier of IP packet: %d\n",ntohs(ip_header ->id ));
    printf("Time To Live: %d\n",ip_header -> ttl );
    printf("Protocol type: %d\n",ip_header -> protocol );
    printf("IP header checksum: %d\n",ip_header -> check );
    printf ("Sender IP address: %u.%u.%u.%u \n", sa[0],sa[1],sa[2],sa[3]);
    printf("Dest . IP address:%u.%u.%u.%u \n", da[0],da[1],da[2],da[3]);
}

unsigned char * create_udphdr (
    unsigned short sp,
    unsigned short dp ,
    unsigned short len ,
    unsigned short checksum)
{
    struct udphdr * udp_header ;
    udp_header = ( struct udphdr *) malloc(sizeof(struct udphdr ));
    udp_header -> source = sp;
    udp_header -> dest = dp ;
    udp_header -> len = len ;
    udp_header -> check = checksum;
    return ((unsigned char *) udp_header );
}

void print_udphdr(unsigned char * udp_headerstr )
{
    struct udphdr * udp_header ;
    udp_header = ( struct udphdr *) malloc(sizeof(struct udphdr ));
    memcpy(udp_header,udp_headerstr,sizeof(struct udphdr ));
    printf("Source UDP port: %d\n",ntohs(udp_header -> source ));
    printf("Destination UDP port: %d\n",ntohs(udp_header -> dest ));
    printf("UDP packet length: %d\n",ntohs(udp_header -> len ));
    printf("UDP header checksum: %d\n",ntohs(udp_header -> check ));
}


unsigned char * create_tcphdr (
    unsigned short sp,
    unsigned short dp ,
    unsigned int seq ,
    unsigned int ack ,
    unsigned char resoff ,
    unsigned char flags,
    unsigned short window,
    unsigned short checksum,
    unsigned short urgp )
{
    struct tcphdr * tcp_header;
    tcp_header = ( struct tcphdr *) malloc(sizeof(struct tcphdr ));
    tcp_header -> source = sp;
    tcp_header -> dest = dp ;
    tcp_header -> seq = seq;
    tcp_header -> ack_seq = ack ;
    tcp_header -> window = window;
    tcp_header -> check = checksum;
    tcp_header -> urg_ptr = urgp ;
    return ((unsigned char *) tcp_header );
}

void print_tcphdr(unsigned char * tcp_headerstr )
{
    struct tcphdr * tcp_header;
    tcp_header = ( struct tcphdr *) malloc(sizeof(struct tcphdr ));
    memcpy(tcp_header,tcp_headerstr,sizeof(struct tcphdr ));
    printf ("Source TCP port: %u\n", ntohs ( tcp_header -> source));
    printf ("Destination TCP port: %u\n", ntohs ( tcp_header -> dest ));
    printf("TCP sequence number: %u\n",ntohl(tcp_header -> seq ));
    printf("TCP acknowledge number: %u\n",ntohl(tcp_header -> ack_seq ));
    printf(" \n ");
    printf("TCP window: %u\n",ntohs(tcp_header -> window ));
    printf("TCP checksum: %u\n",ntohs(tcp_header -> check ));
}

