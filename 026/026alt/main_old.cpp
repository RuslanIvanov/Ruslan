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

#define TYPE0  0
#define TYPE8  8

#define TYPE11  11
#define TYPE13  13
#define TYPE14  14

#define LENDATA 64

void out(int sig=0);
bool bOut = false;
int port = 0;
char addrDest[BUFSIZ]="";
bool mytrecerout(void* buf,int bytes_read);
char hostIp[16*2]="";

// ICMP Header - RFC 792
struct ICMP_HEADER
{
	unsigned char type;
	unsigned char code;
	unsigned short crc;
	unsigned short id;
	unsigned short seq;
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
	struct	in_addr addrSrc; /*struct in_addr {unsigned long s_addr;};*/
	struct	in_addr addrDst;
};

struct ECHO_REPLY
{
	IP_HEADER   ipHeader;
	ICMP_HEADER icmpHeader;
        /*unsigned int current_time;
	unsigned int in_time;
	unsigned int out_time;*/
	unsigned long long time;
	unsigned char data[LENDATA];
};

struct ECHO_REQUEST
{
	IP_HEADER   ipHeader;
	ICMP_HEADER icmpHeader;
	/*unsigned int current_time;
	unsigned int in_time;
	unsigned int out_time;*/
	unsigned long long time;
	unsigned char data[LENDATA];
};

struct  TRACE_INFO
{
        int packetid;
        int ttl;
        int proto;
        int size;
        unsigned long saddr;
        unsigned long daddr;
};

ECHO_REQUEST echoReq;
ECHO_REPLY  echoRpl;
TRACE_INFO trace;

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
    	while((opt= getopt(argc, argv, "ht:l:")) != -1)
    	switch(opt)
    	{
        	case 't': sscanf(optarg,"%d",&timePing); break;
		case 'l': strcpy(hostIp,""); sscanf(optarg,"%s",hostIp); break; 
                case 'h':
	        	printf("\nargv:\n");
        	        printf("\t-t\t time ping mc\n");
			printf("\t-l\t host ip address\n");
			return 0;
        }

	printf("\ntime period for ping: %d mc, host ip %s\n",timePing, hostIp);

	strcpy(addrDest,argv[argc-1]);

	struct sockaddr_in addr;
	
	/*struct sockaddr_in 
	{
   		short            sin_family;   // e.g. AF_INET
    		unsigned short   sin_port;     // e.g. htons(3490)
    		struct in_addr   sin_addr;     // see struct in_addr, below
    		char             sin_zero[8];  // zero this if you want to
	};*/	

	struct in_addr adrDst; /*struct in_addr {unsigned long s_addr;};*/
	struct in_addr adrHost;

	memset(&addr,0,sizeof(struct sockaddr_in));
	memset(&adrDst,0,sizeof(struct in_addr));
	memset(&adrHost,0,sizeof(struct in_addr));

	printf("create sock\n");
	int raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if(raw_sock < 0)
	{perror("socket"); return 0;}

	if(inet_pton(AF_INET, addrDest,(void*)&adrDst)<=0)
	{
		perror("Error destination ip: ");

		struct hostent* phost  = gethostbyname(addrDest); //для www.google.ru
		if (phost == NULL) {perror("gethostbyname"); printf("\nError in destination address!\n"); return 0;}

		printf("Destination IP address : %s\n", inet_ntoa(*(struct in_addr*)phost->h_addr));
		adrDst = *(struct in_addr*)phost->h_addr;
	}

	if(inet_pton(AF_INET, hostIp,(void*)&adrHost)<=0)
	{
		perror("Error host ip: ");
		strcpy(hostIp,"");
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if(strcmp(hostIp,"")==0)
	{
		addr.sin_addr.s_addr = adrHost.s_addr;// адрес хоста
		char hostname[BUFSIZ];//назначить ip 
		if (gethostname(hostname, sizeof(hostname)) == -1) 
  		{perror("gethostname");return 0;}

		struct hostent* plocalhost  = gethostbyname(hostname);
		if (plocalhost == NULL) {perror("gethostbyname");}
		printf("IP address: %s, hostname %s\n", inet_ntoa(*(struct in_addr*)plocalhost->h_addr), hostname);//*/
	}

	printf(" addr host %ld",adrHost.s_addr);
	printf("setsockopt\n");

	// provide our own IP header and not let the kernel provide one:
	int optval = 1;
	if(setsockopt(raw_sock,IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval))==-1){ perror("setsockopt"); }

	if(bind(raw_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{perror("bind");return 0;}

	memset((void*)&echoReq,0,sizeof(struct ECHO_REQUEST));
	memset((void*)&echoRpl,0,sizeof(struct ECHO_REPLY));

	echoReq.ipHeader.VIHL=4<<4|(0x0f&5); //Ver 0-4, HLEN 4-8
	echoReq.ipHeader.TOS=0;
	echoReq.ipHeader.totLen = htons(sizeof(struct ECHO_REQUEST));
	echoReq.ipHeader.id = getpid();
	echoReq.ipHeader.flagOff=0;
	echoReq.ipHeader.TTL=1;
	echoReq.ipHeader.protocol = IPPROTO_ICMP;
	echoReq.ipHeader.crc=0;//ядро заполнит

	echoReq.ipHeader.addrSrc.s_addr = adrHost.s_addr;
	echoReq.ipHeader.addrDst.s_addr = adrDst.s_addr;

	int iConnect=1;
	while(bOut==false)
	{
		echoReq.icmpHeader.type= TYPE8;//TYPE13;
		echoReq.icmpHeader.code = 0;
		echoReq.icmpHeader.crc = 0;
		echoReq.icmpHeader.id = getpid();
		echoReq.icmpHeader.seq = iConnect;
		echoReq.time = getTickCount();

		memset((void*)echoReq.data,0,LENDATA);
		echoReq.icmpHeader.crc = crcIcmp((unsigned short *)&echoReq,sizeof(struct ECHO_REQUEST));

		memset(&addr,0,sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr =adrDst.s_addr;

		int nSend = sizeof(struct ECHO_REQUEST);
		ssize_t nsend = sendto(raw_sock, &echoReq, nSend, MSG_DONTWAIT,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
		if(nsend ==-1) {perror("sendto"); break;}

		printf("%d bytes from  host ", nsend);

		struct sockaddr_in addrFrom;
		int nAddrLen = sizeof(struct sockaddr_in);
		int bytes_read  = recvfrom(raw_sock, &echoRpl, sizeof(struct ECHO_REPLY), 0,(struct sockaddr*)&addrFrom,(socklen_t*)&nAddrLen);
		if(bytes_read ==-1) {perror("recvfrom"); break;}

		bool bTrace = mytrecerout(&echoRpl,bytes_read);
		if(bTrace){printf(" trace is OK!");}
		else printf(" time exceeded in transit");
		printf("\n");
		iConnect++;
		delay(timePing);
	}

	close(raw_sock);

	printf("\nExit...\n");
	return 0;
}

bool mytrecerout(void *pbuf,int bytes_read)
{
	struct ECHO_REPLY *preply = (struct ECHO_REPLY *)pbuf;

	if( preply->ipHeader.protocol != IPPROTO_ICMP ) return  false;

	if(preply->icmpHeader.type == TYPE14 || preply->icmpHeader.type == TYPE0)
	{
		char host[16*2]=""; char hostDst[16*2]="";
		if(inet_ntop(AF_INET, &(preply->ipHeader.addrSrc),&host[0], 16*2)==NULL)
			perror("Error reply ip");
		if(inet_ntop(AF_INET, &(preply->ipHeader.addrDst),&hostDst[0], 16*2)==NULL)
			perror("Error reply ip");

		else printf("ttl = %d,  RTT = %Ld, seq %d,remote host %s send to %s",preply->ipHeader.TTL,preply->time,preply->icmpHeader.seq,host,hostDst);
		return true;
	}else if(preply->icmpHeader.type == TYPE11)
		{
			printf(" время (ttl) истекло");
		}else return false;
}

void out(int sig)
{
    if( sig==SIGTERM || sig == SIGINT )
    {
	printf("\nGoodbye 'ping'\n");
	bOut=true;
    }
}
