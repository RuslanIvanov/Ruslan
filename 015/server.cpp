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
#include <vector>
#include <string>

#define MAX_CONNECT 100
unsigned int port;
void out(int sig=0);
bool bOut = false;
char nameServ[BUFSIZ];
void * funcThread(void*);

struct Param
{
      struct sockaddr_in addr;
      int sockTh;
      int n;
};

int main(int argc, char* argv[])
{

    signal (SIGTERM, out);
    signal (SIGINT, out);

    struct sockaddr_in addr;
    struct sockaddr_in addrCli[MAX_CONNECT];
    memset(&addr,0,sizeof(struct sockaddr_in));
    memset(&addrCli,0,sizeof(struct sockaddr_in)*MAX_CONNECT);

    if(argc==2)
    {
	port = atoi(argv[1]);
	if(port<=0){printf("\nError number port"); return 0;}

	addr.sin_family = AF_INET;
    	addr.sin_port = htons(port); 
	
	strcpy(nameServ,argv[2]); 
    }else { printf("\nSet command string: 'port' 'name'"); return 0; }

    int sock, listener;

    pthread_t thId[MAX_CONNECT];
    Param param[MAX_CONNECT];
    char buf[BUFSIZ];
    int bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    listen(listener, MAX_CONNECT);
    int iConnect=0;
    while(bOut==false)
    {
	if(iConnect>=MAX_CONNECT) continue;

	int nAddrCli=0;
        sock = accept(listener, (struct sockaddr*)&addrCli[iConnect], (socklen_t *)&nAddrCli);
        if(sock < 0)
        {
            perror("accept");
            return 0;
        }
	
	thId[iConnect] = pthread_create(&thId[iConnect], NULL, funcThread, &param[iConnect]);

	iConnect++;
      
    }
    
	printf("\nWait threads...");
	for(int i=0;i<iConnect;i++)
	{
		pthread_join(thId[i],NULL);
		pthread_cancel(thId[i]);
	}

    return 0;
}

void * funcThread(void* pVoid)
{
	int bytes_read=0;
	Param *pparam = (Param*)pVoid;

	while(bOut==false)
        {
	    char buf[BUFSIZ]={'\0'};
            bytes_read = recv(pparam->sockTh, buf, BUFSIZ, 0);
            if(bytes_read <= 0) continue;

            send(pparam->sockTh, buf, bytes_read, 0);
        }
    
        close(pparam->sockTh);
	return 0;
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye server'\n");
	bOut=true;
    }
}
