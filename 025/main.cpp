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
int port = 12345;

bool myping(char* buf,int bytes_read);
bool myproto(char*,int *);

struct icmp_header
{
	unsigned char type;
	unsigned char code;
	unsigned short crc;
	unsigned short id;
	unsigned short number;
	unsigned char data[64];
};

icmp_header header_in;
icmp_header header_out;

int main(int argc, char* argv[])
{
    signal (SIGTERM, out);
    signal (SIGINT, out);

    struct sockaddr_in addr;
    struct in_addr adrDst;


    memset(&addr,0,sizeof(struct sockaddr_in));
    memset(&adrDst,0,sizeof(struct in_addr));

    char bufOut[BUFSIZ];
    char bufIn[BUFSIZ];

    int raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);//IPPROTO_RAW);
    if(raw_sock < 0)
    {perror("socket"); return 0;}

    if(inet_pton(AF_INET, argv[1],(void*)&adrDst)<=0)
    {perror("Error ip: "); return 0;}


    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = /*htonl*/adrDst.s_addr; // адрес хоста arg[1]

    int optval = 1;
    if(setsockopt(listener, SOL_SOCKET, IP_HDRINCL, &optval, sizeof optval)==-1){perror("SO_REUSEADDR:"); }

   header_out.type=0;
   header_out.code = 8;
   header_out.crc = 0;
   header_out.id = getpid();
   header_out.number = 0;

   memcpy(header_in,0,sizeof(struct icmp_header));
   header_in.type=0;
   header_in.code = 0;

    int iConnect=0;
    while(bOut==false)
    {
	int nSend;

	ssize_t nsend = sendto(raw_sock, bufOut, nSend, MSG_DONTWAIT,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));

	header_out.number = iConnect;


	int bytes_read  = recvfrom(raw_sock, bufIn, BUFSIZ, 0, NULL, NULL);

	myping(bufIn,bytes_read);

	iConnect++;
    }

   close(raw_sock);

   return 0;
}

bool myproto(char* pbuf,int* pn)
{
	return true;
}

bool myping(char *pbuf,int bytes_read)
{
	return true;
}

void out(int sig)
{
    if( sig==SIGTERM || sig == SIGINT )
    {
	printf("\nGoodbye 'ping'\n");
	bOut=true;
    }
}
