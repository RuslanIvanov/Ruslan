#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <vector>
#include <string>
#include <map>

using namespace std;

#define MAX_CONNECT 100

void out(int sig=0);
bool bOut = false;
int port = 0;

bool myping(void* buf,int bytes_read);
bool myproto(void*,int *);

// ICMP Header - RFC 792
struct ICMP_HEADER
{
	unsigned char type;
	unsigned char code;
	unsigned short crc;
	unsigned short id;
	unsigned short nPack;
//	unsigned char nData;
};

struct ECHO_REQUEST
{
	ICMP_HEADER icmpHeader;
	unsigned int time;
	char data[64];
};

// IP Header -- RFC 791
struct IP_HEADER
{
	unsigned char   VIHL;			
	unsigned char	TOS;			
	unsigned short	totLen;			
	unsigned short	id;			
	unsigned short	flagOff;		
	unsigned char	TTL;			
	unsigned char	protocol;		
	unsigned short	crc;		
	struct	in_addr addrSrc;
	struct	in_addr addrDst;	
};

struct ECHO_REPLY
{
	IP_HEADER	ipHeader;
	ECHO_REQUEST	echoRequest;
	char    cFiller[BUFSIZ];
};

ECHO_REQUEST echoReq;
ECHO_REPLY  echoRpl;

unsigned short crcIcmp(unsigned short *addr, int len)
{
	int nleft = len;
	unsigned short answer;
	int sum = 0;
	
	while( nleft > 1 )  
	{
		sum += *addr++;
		nleft -= 1;
	}

	sum += (sum >> 16);			
	answer = ~sum;				
	return answer;
}

unsigned int getTickCount()
{
	return 0;
}

int main(int argc, char* argv[])
{
    signal (SIGTERM, out);
    signal (SIGINT, out);

    if(argc != 2)
    {
	printf("\nError: set ip in command string!\n");
	return 0;
    }

    struct sockaddr_in addr;
    struct in_addr adrDst;

    memset(&addr,0,sizeof(struct sockaddr_in));
    memset(&adrDst,0,sizeof(struct in_addr));

    int raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(raw_sock < 0)
    {perror("socket"); return 0;}

    if(inet_pton(AF_INET, argv[1],(void*)&adrDst)<0)
    {perror("Error ip: "); return 0;}

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = /*htonl*/adrDst.s_addr; // адрес хоста 

    int optval = 1;

    if(setsockopt(raw_sock,IPPROTO_IP, IP_HDRINCL, &optval, sizeof optval)==-1){perror("setsockopt"); }

   memcpy(&echoReq,0,sizeof(struct ECHO_REQUEST));
   echoReq.icmpHeader.type=8;
   echoReq.icmpHeader.code = 0;
   echoReq.icmpHeader.crc = 0;
   echoReq.icmpHeader.id = getpid();
   echoReq.icmpHeader.nPack = 1;

   memcpy(&echoRpl,0,sizeof(struct ECHO_REPLY));
   echoRpl.echoRequest.icmpHeader.type=0;
   echoRpl.echoRequest.icmpHeader.code = 0;

    int iConnect=1;
    while(bOut==false)
    {
	printf(" while \n");
	int nSend = sizeof(struct ECHO_REQUEST);

	echoReq.icmpHeader.nPack = iConnect;
	echoReq.time = getTickCount();
	memcpy(echoReq.data,0,64);
	echoReq.icmpHeader.crc = crcIcmp((unsigned short *)&echoReq,sizeof(struct ECHO_REQUEST));

	ssize_t nsend = sendto(raw_sock, &echoReq, nSend, MSG_DONTWAIT,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if(nsend ==-1) {perror("sendto"); break;}

	printf("\n%d bytes from '%s' ", nsend,argv[1]);	

	struct sockaddr_in addr;
	int nAddrLen = sizeof(struct sockaddr_in);
	int bytes_read  = recvfrom(raw_sock, &echoRpl, sizeof(struct ECHO_REPLY), 0,(struct sockaddr*)&addr,(socklen_t*)&nAddrLen);
	if(bytes_read ==-1) {perror("recvfrom"); break;}

	bool bPing = myping(&echoRpl,bytes_read);
	if(bPing){printf(" ping is OK!");}
	else printf(" Destination Host Unreachable");
	printf("\n");
	iConnect++;
    }

   close(raw_sock);

   printf("\nExit...\n");
   return 0;
}

bool myproto(void* pbuf,int* pn)
{
	return true;
}

bool myping(void *pbuf,int bytes_read)
{
	struct ECHO_REPLY *preply = (struct ECHO_REPLY *)pbuf;
	if(preply->echoRequest.icmpHeader.type==8)
	{
		char host[16*2]="";
		if(inet_ntop(AF_INET, &(preply->ipHeader.addrSrc),&host[0], 16*2)==NULL)
			perror("Error reply ip");
		else printf("ttl = %d, host %s",preply->ipHeader.TTL,host);
		return true;
	}else return !true;
}

void out(int sig)
{
    if( sig==SIGTERM || sig == SIGINT )
    {
	printf("\nGoodbye 'ping'\n");
	bOut=true;
    }
}
