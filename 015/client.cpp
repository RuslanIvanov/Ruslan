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

char nameClient[BUFSIZ];
unsigned int port;
void out(int sig=0);
bool bOut = false;
int sockfd;

int main(int argc, char* argv[])
{
    signal (SIGTERM, out);
    signal (SIGINT, out);

    struct sockaddr_in addr;
    struct in_addr adrDst;
    memset(&addr,0,sizeof(struct sockaddr_in));
    memset(&adrDst,0,sizeof(struct in_addr));

    if(argc>1 && argc<=4)
    {
	if(inet_pton(AF_INET, argv[1],(void*)&adrDst)<=0)
	{perror("Error ip: "); return 0;}

	port = atoi(argv[2]);
	if(port<=0){printf("\nError number port"); return 0;}

	addr.sin_family = AF_INET;
    	addr.sin_port = htons(port); 
	addr.sin_addr = adrDst;
	strcpy(nameClient,argv[3]); 
    }else { printf("\nSet command string: 'ip-addres' 'port' 'name'\n"); return 0; }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0)
    {
        perror("socket");
        return 0;
    }
	
//	int optval = 1;
 //       setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    printf("\nWait client ' %s ' connect...", nameClient);
    if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        return 0;
    }

    while(bOut==false)
    {
		char buf[BUFSIZ] = {'\0'};

		printf("\nsend msg client  ' %s ': ",nameClient);
		fgets(buf,BUFSIZ,stdin);
		sprintf(buf,"%s:%s",nameClient,buf);
		int i = strlen(buf)-1;

		if(buf[i] == '\n') buf[i]= '\0';

    		send(sockfd, buf, strlen(buf), 0);

    		recv(sockfd, buf, sizeof(buf), 0);
		
		printf("\n\tread msg: %s",buf);		
    }

    shutdown(sockfd, 2);
    close(sockfd);
    printf("\nExit client ' %s \n",nameClient);
    return 0;
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye ' %s '\n",nameClient);
	bOut=true;
	shutdown(sockfd, 2);
	close(sockfd);
	exit(0);
    }
}
