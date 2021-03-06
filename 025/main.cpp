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
#include <netdb.h>
using namespace std;

#define MAX_CONNECT 100

void out(int sig=0);
bool bOut = false;
int port = 0;
char addrDest[BUFSIZ]="";
bool myping(void* buf,int bytes_read);

double dtime;
double timeSend;

// ICMP Header - RFC 792
struct ICMP_HEADER
{
	unsigned char type;
	unsigned char code;
	unsigned short crc;
	unsigned short id;
	unsigned short seq;
};

struct ECHO_REQUEST
{
	ICMP_HEADER icmpHeader;
	unsigned long long time;
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
	IP_HEADER   ipHeader;
	ICMP_HEADER icmpHeader;
        unsigned long long time;
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
	return  clock();
}

void delay(int mc)
{
    usleep(mc*1000);// suspend execution for microsecond intervals
}

int main(int argc, char* argv[])
{
	signal (SIGTERM, out);
	signal (SIGINT, out);

	if(argc <= 1)
	{
		printf("\nError: set ip in command string! Help: ./main -h\n");
		return 0;
	}

	int opt; int timePing=1000;
    	while((opt= getopt(argc, argv, "ht:")) != -1)
    	switch(opt)
    	{
        	case 't': sscanf(optarg,"%d",&timePing); break; 
                case 'h':
	        	printf("\nargv:\n");
        	        printf("\t-t\t time ping mc\n");
			return 0;
        }

	printf("\ntime period for ping: %d mc\n",timePing);

	strcpy(addrDest,argv[argc-1]);

	struct sockaddr_in addr;
	struct in_addr adrDst;

	memset(&addr,0,sizeof(struct sockaddr_in));
	memset(&adrDst,0,sizeof(struct in_addr));

	printf("create sock\n");
	int raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if(raw_sock < 0)
	{perror("socket"); return 0;}

	if(inet_pton(AF_INET, addrDest,(void*)&adrDst)<=0)
	{
		perror("Error ip: ");

		struct hostent* phost  = gethostbyname(addrDest); //для www.google.ru
		if (phost == NULL) {perror("gethostbyname"); printf("\nError in destination address!\n"); return 0;}

		printf("Destination IP address : %s\n", inet_ntoa(*(struct in_addr*)phost->h_addr));
		adrDst = *(struct in_addr*)phost->h_addr;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htons(INADDR_ANY);//adrDst.s_addr; // адрес хоста 

	printf("setsockopt\n");

	// int optval = 1;
	// if(setsockopt(raw_sock,IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval))==-1){perror("setsockopt"); }

	if(bind(raw_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{perror("bind");return 0;}

	memset((void*)&echoReq,0,sizeof(struct ECHO_REQUEST));
	memset((void*)&echoRpl,0,sizeof(struct ECHO_REPLY));

	/*struct hostent* phost  = gethostbyname("localhost");
	if (phost == NULL) {perror("gethostbyname");}
	printf("IP address: %s\n", inet_ntoa(*(struct in_addr*)phost->h_addr));*/

	int iConnect=1;
	while(bOut==false)
	{
	echoReq.icmpHeader.type=8;
	echoReq.icmpHeader.code = 0;
	echoReq.icmpHeader.crc = 0;
	echoReq.icmpHeader.id = getpid();
	echoReq.icmpHeader.seq = iConnect;
	echoReq.time = getTickCount();

	memset((void*)echoReq.data,0,64);
	echoReq.icmpHeader.crc = crcIcmp((unsigned short *)&echoReq,sizeof(struct ECHO_REQUEST));

	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = /*htonl*/(adrDst.s_addr);

	int nSend = sizeof(struct ECHO_REQUEST);
	ssize_t nsend = sendto(raw_sock, &echoReq, nSend, MSG_DONTWAIT,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if(nsend ==-1) {perror("sendto"); break;}

	printf("%d bytes from  host ", nsend);

	struct sockaddr_in addrFrom;
	int nAddrLen = sizeof(struct sockaddr_in);
	int bytes_read  = recvfrom(raw_sock, &echoRpl, sizeof(struct ECHO_REPLY), 0,(struct sockaddr*)&addrFrom,(socklen_t*)&nAddrLen);
	if(bytes_read ==-1) {perror("recvfrom"); break;}

	bool bPing = myping(&echoRpl,bytes_read);
	if(bPing){printf(" ping is OK!");}
	else printf(" Destination Host Unreachable");
	printf("\n");
	iConnect++;
	delay(timePing);
	}

	close(raw_sock);

	printf("\nExit...\n");
	return 0;
}

bool myping(void *pbuf,int bytes_read)
{
	struct ECHO_REPLY *preply = (struct ECHO_REPLY *)pbuf;
	if(preply->icmpHeader.type==0 && preply->ipHeader.protocol == IPPROTO_ICMP)
	{
		char host[16*2]=""; char hostDst[16*2]="";
		if(inet_ntop(AF_INET, &(preply->ipHeader.addrSrc),&host[0], 16*2)==NULL)
			perror("Error reply ip");
		if(inet_ntop(AF_INET, &(preply->ipHeader.addrDst),&hostDst[0], 16*2)==NULL)
			perror("Error reply ip");

		else printf("ttl = %d,  RTT = %Ld, seq %d,remote host %s send to %s",preply->ipHeader.TTL,(getTickCount()-preply->time),preply->icmpHeader.seq,host,hostDst);
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
