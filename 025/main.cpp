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

int main(int argc, char* argv[])
{

    signal (SIGTERM, out);
    signal (SIGINT, out);

    struct sockaddr_in addr;

    memset(&addr,0,sizeof(struct sockaddr_in));

    char bufOut[BUFSIZ];
    char bufIn[BUFSIZ];

    int raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(raw_sock < 0)
    {perror("socket"); return 0;}

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //int optval = 1;
    //if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)==-1){perror("SO_REUSEADDR:"); }
 
    int iConnect=0;
    while(bOut==false)
    {
	int nSend;

	ssize_t nsend = sendto(raw_sock, bufOut, nSend, MSG_DONTWAIT,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));

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
