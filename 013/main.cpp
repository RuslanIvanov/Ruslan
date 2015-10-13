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

void out(int sig=0);
void * funcThread(void*);
bool bOut = false;
int ports[2];
char iplocalhost[]="127.0.0.1";
int main(int argc,char* argv[], char** env)
{

    signal (SIGTERM, out);
    signal (SIGINT, out);

    if(argc==3)
    {
	char buf[BUFFER_SIZE];

	port[0] = atoi(argv[1]);
	port[1] = atoi(argv[2]);

	printf("\nfifo localport ' %s ', fifo remoteport ' %s ', exit ' Ctrl+C '\n",argv[1],argv[2]);

	pthread_t thId = 0;
	pthread_create(&thId, NULL, funcThread, NULL);
	printf("\nWait start task...");
	sleep(1);
		
	int sockfd = socket(AF_INET, SOCK_DGRAM ,0);
	if(sockfd==-1)
	{
		perror("socket:");return 0;
	}
	struct sockaddr_in dest_addr;

	memset(&dest_addr, 0, sizeof(sockaddr_in));

        dest_addr.sa_family = AF_INET;
	dest_addr.sinport =  htons(port[1]); 
	dest_addr.s_addr = htonl(iplocalhost);
	
	strncpy(addr.sun_path, lport, sizeof(addr.sun_path) - 1);

	int rez = bind(sockfd,&dest_addr,sizeof(struct sockaddr_in));
	if(rez!=0){perror("bind: "); return 0;}
	
	while(bout==false)
	{
		char buf[BUFSIZ] = '\0';
		printf("\nsend msg");
		gets(buf);

		ssize_t nsend = sendto(sockfd, buf, sizeof(buf),0, &dest_addr, sizeof(struct sockaddr_in));
	}

	pthread_join(thId,NULL);
 
    }else {printf("\nError command string! Enter: 'pipein' 'pipeout'.\n");}
    printf("\n\nExit...\n");
   
    return 0;
}

void * funcThread(void* )
{
    printf("\nRUN TASK READ");
  

    printf("\nEXIT TASK");
    return 0;
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye chat udp(signal %d)\n",sig);

	bOut=true;
	exit(0);
    }
}
