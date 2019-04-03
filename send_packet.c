#include <stdio.h>
//#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"


/*int get_if_name_index(int sock, char *if_name)
{
	struct ifreq ifreq_i;
	memset(&ifreq_i, 0, sizeof(ifreq_i));
	strncpy(ifreq_i.ifi_name, if_name, IFNAMSIZ-1);
	
	if ((ioctl(sock, SIOCGIFINDEX, &ifreq_i)) < 0) {
		printf("error get if_name index\n");
		return -1;
	} 
	printf("get if_name %s, index %d\n", if_name, ifreq_i.ifr_ifindex);


}*/
unsigned short checksum(unsigned short* buff, int _16bitword)
{
	unsigned long sum;
	for(sum=0;_16bitword>0;_16bitword--)
		sum+=htons(*(buff)++);
	sum = ((sum >> 16) + (sum & 0xFFFF));
	sum += (sum>>16);
	return (unsigned short)(~sum);
}




int main()
{
	int sock;
	int index;
	char *sendbuff;
	int total_len = 0;
	sock = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
	if (socket < 0) {
		printf("error in socket\n");
		return -1;
	}	
	index = if_nametoindex("ens33");
	printf("index %d\n", index);

	sendbuff=(unsigned char*)malloc(64); // increase in case of more data
	memset(sendbuff,0,64);

	struct ethhdr *eth = (struct ethhdr *)(sendbuff);

	eth->h_source[0] = 0x00;
	eth->h_source[1] = 0x0c;
	eth->h_source[2] = 0x29;
	eth->h_source[3] = 0xfe;
	eth->h_source[4] = 0x10;
	eth->h_source[5] = 0x5f;

	eth->h_dest[0] = 0xdc;
	eth->h_dest[1] = 0x08;
	eth->h_dest[2] = 0x56;
	eth->h_dest[3] = 0xa2;
	eth->h_dest[4] = 0x01;
	eth->h_dest[5] = 0x10;
	eth->h_proto = htons(ETH_P_IP);
	total_len += sizeof(struct ethhdr);
	printf("mac: " MACSTR "\n", MAC2STR(eth->h_source));


	struct iphdr *iph = (struct iphdr*)(sendbuff + sizeof(struct ethhdr));
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 16;
	iph->id = htons(10201);
	iph->ttl = 64;
	iph->protocol = 17;
	iph->saddr = inet_addr("172.16.102.48");
	iph->daddr = inet_addr("172.16.102.42"); // put destination IP address
 	printf("saddr: %d\n", iph->saddr);
	total_len += sizeof(struct iphdr);


	struct udphdr *uh = (struct udphdr *)(sendbuff + sizeof(struct iphdr) + sizeof(struct ethhdr));
 
	uh->source = htons(23451);
	uh->dest = htons(23452);
	uh->check = 0;
 
	total_len+= sizeof(struct udphdr);

	sendbuff[total_len++] = 0xAA;
	sendbuff[total_len++] = 0xBB;
	sendbuff[total_len++] = 0xCC;
	sendbuff[total_len++] = 0xDD;
	sendbuff[total_len++] = 0xEE;

	uh->len = htons((total_len - sizeof(struct iphdr) - sizeof(struct ethhdr)));
	//UDP length field
	iph->tot_len = htons(total_len - sizeof(struct ethhdr));
	//IP length field

	struct sockaddr_ll sadr_ll;
	sadr_ll.sll_ifindex = index; // index of interface
	sadr_ll.sll_halen = ETH_ALEN; // length of destination mac address
	sadr_ll.sll_addr[0] = 0xdc;
	sadr_ll.sll_addr[1] = 0x08;
	sadr_ll.sll_addr[2] = 0x56;
	sadr_ll.sll_addr[3] = 0xa2;
	sadr_ll.sll_addr[4] = 0x01;
	sadr_ll.sll_addr[5] = 0x10;
	int send_len = 0;
	send_len = sendto(sock, sendbuff, 64, 0, (const struct sockaddr*)&sadr_ll,sizeof(struct sockaddr_ll));
	if (send_len < 0) {
	return -1;
 
	}
	
	printf("sendlen=%d\n", send_len);








}

