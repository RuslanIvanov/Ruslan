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
#include <netinet/in.h>

void out(int sig=0);
void * funcThread(void*);
bool bOut = false;
int ports[2];

int main(int argc,char* argv[], char** env)
{

    signal (SIGTERM, out);
    signal (SIGINT, out);

    if(argc==3)
    {
	char buf[BUFSIZ];

	ports[0] = atoi(argv[1]);
	ports[1] = atoi(argv[2]);

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

        dest_addr.sin_family = AF_INET;
	dest_addr.sin_port =  htons(ports[1]); 
	dest_addr.sin_addr.s_addr  = htonl(INADDR_LOOPBACK);

	while(bOut==false)
	{
		char buf[BUFSIZ] = {'\0'};

		printf("\nsend msg: ");
		fgets(buf,BUFSIZ,stdin);

		int i = strlen(buf)-1;
		if(buf[i] == '\n') buf[i]= '\0';

		ssize_t nsend = sendto(sockfd, buf, i, MSG_DONTWAIT,(struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));

		printf(" sended[%d]\n",,nsend);
	}

	close(sockfd);
	pthread_join(thId,NULL);
 
    }else {printf("\nError command string! Enter: 'port send' 'port recv'.\n");}
    printf("\n\nExit...\n");
   
    return 0;
}

void * funcThread(void* )
{
    	printf("\nRUN TASK READ");
   	char buf[BUFSIZ]={'\n'};

	int sockfd = socket(AF_INET, SOCK_DGRAM ,0);
	if(sockfd==-1)
	{
		perror("socket thread:");return 0;
	}
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(sockaddr_in));

        addr.sin_family = AF_INET;
	addr.sin_port =  htons(ports[0]); 
	addr.sin_addr.s_addr  = htonl(INADDR_LOOPBACK);

	if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    	{
        	perror("bind");
        	return 0;
    	}
   	while(bOut == false)
   	{
		int rez=0;
		do
		{
			rez = recvfrom(sockfd, buf, BUFSIZ, 0, NULL, NULL);
        		buf[rez] = '\0';
			printf("\nrecv[%d]: %s",strlen(buf),buf);
		}
		while(rez>0);
   	}
	close(sockfd);
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

