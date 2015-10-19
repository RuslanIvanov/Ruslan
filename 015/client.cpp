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
pthread_t thId = 0;
void * funcThread(void*);

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
	printf("\n set ip: %d",adrDst.s_addr);
	addr.sin_addr.s_addr = /*htonl*/(adrDst.s_addr);
	strcpy(nameClient,argv[3]); 
    }else { printf("\nSet command string: 'ip-addres' 'port' 'name'\n"); return 0; }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0)
    {
        perror("socket");
        return 0;
    }

    pthread_create(&thId, NULL, funcThread, NULL);

    int rez=-1;
    do
    {	
    	printf("\nWait client ' %s ' connect...", nameClient);
	rez = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    	if(rez < 0) {perror("connect");sleep(1);}

	if(bOut==true) break;
    }
    while(rez < 0);	

    while(bOut==false)
    {
		char buf[BUFSIZ] = {'\0'};
		char bufout[BUFSIZ] = {'\0'};
		const char *cmd[3]= {"","cli","msg10"};
		int indCmd=0;	
		printf("\nMenu");
		printf("\n\tget names clients - enter 1;");
		printf("\n\tget last ten messages - enter 2;");
		printf("\n\tsend message - enter 0;");
		scanf("%d",&indCmd);
		if(indCmd==0)
		{
			printf("\nmsg ' %s ': ",nameClient);
			fgets(buf,BUFSIZ,stdin);
			int i = strlen(buf)-1;
			if(buf[i] == '\n') buf[i]= '\0';
			sprintf(bufout,"$N=%s:$C=%s:$M=%s",nameClient,cmd[indCmd],buf);
		}else if(indCmd==1 || indCmd==2)
			{sprintf(bufout,"$N=%s:$C=%s:$M=",nameClient,cmd[indCmd]);}
		else {printf("\nError command..."); continue;}

    		send(sockfd, bufout, strlen(bufout), 0);
    }

    shutdown(sockfd, 2);
    close(sockfd);
    pthread_cancel(thId);
    pthread_join(thId,NULL);
    printf("\nExit client ' %s \n",nameClient);
    return 0;
}

void * funcThread(void* )
{
    	printf("\nRUN TASK READ");
   	char bufin[BUFSIZ]={'\n'};
	
   	while(bOut == false)
   	{
		int rez=0;
		do
		{
			rez = recv(sockfd, bufin, sizeof(bufin), 0);
        		bufin[rez] = '\0';
			printf("\nrecv[%d]: %s",strlen(bufin),bufin);
		}
		while(rez>0);
   	}
	
    	printf("\nEXIT TASK");
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
