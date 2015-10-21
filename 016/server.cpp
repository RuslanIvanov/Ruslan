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

#define MAX_CONNECT 20

int port=80;
void out(int sig=0);
bool bOut = false;
int iConnect;

void * funcThread(void*);

struct Param
{
      struct sockaddr_in addr;
      int sockTh;
      int n;
};

Param param[MAX_CONNECT];
pthread_t thId[MAX_CONNECT];

int makeResponse(char* response, int nr,const char*);
void head(char *head,int n, bool bOK);

int main(int argc, char* argv[])
{
    signal (SIGTERM, out);
    signal (SIGINT, out);

    if(argc==2)
    {
	port = atoi(argv[1]);
    }

    int sock, listener;
    char buf[BUFSIZ];
    int bytes_read;
    struct sockaddr_in addr;
    struct sockaddr_in addrCli[MAX_CONNECT];
    memset(&addr,0,sizeof(struct sockaddr_in));
    memset(&addrCli,0,sizeof(struct sockaddr_in)*MAX_CONNECT);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int optval = 1;
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)==-1){perror("SO_REUSEADDR:"); }

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    listen(listener, MAX_CONNECT);
    
    iConnect=0;
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

	printf("\nServer connected %d! Create socket %x",iConnect+1,sock);
        param[iConnect].sockTh = sock;
        param[iConnect].addr = addrCli[iConnect];
        param[iConnect].n = nAddrCli;  
        pthread_create(&thId[iConnect], NULL, funcThread, &param[iConnect]);
        sleep(1);
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
        int bConnect = false;
        {
            char buf[BUFSIZ]={'\0'};
            bytes_read = recv(pparam->sockTh, buf, BUFSIZ, 0);
            if(bytes_read <= 0) perror("");
	    
	    printf("%s",buf);	

	   //разобрать заголовок

	    char answer[BUFSIZ+BUFSIZ];
	    head(answer,BUFSIZ+BUFSIZ,true);
	    char response[BUFSIZ];
	    if(makeResponse(response, BUFSIZ,"index.html"))
	    {	
		strcat(answer,response);
		sendto(pparam->sockTh, answer, strlen(answer),0,( struct sockaddr *)(&pparam->addr),pparam->n);	    
	    } else if(makeResponse(response, BUFSIZ,"404_Not_Found.html"))
		    {
                        strcat(answer,response);
			sendto(pparam->sockTh, answer, strlen(answer),0,( struct sockaddr *)(&pparam->addr),pparam->n);
	            }

	}
        shutdown(pparam->sockTh, 2);
        close(pparam->sockTh);
        printf("\nclose socket %d",pparam->sockTh);

        return 0;
}

void head(char *head,int n, bool bOK)
{ 
	strcpy(head,"");
	strcat(head,"HTTP/1.1 200 OK\r\n");
        strcat(head, "Version: HTTP/1.1\r\n");
        strcat(head, "Content-Type: text/html; charset=utf-8\r\n");
        strcat(head, "Content-Length: ");char buf[BUFSIZ]; sprintf(buf,"%i",n);
	strcat(head, buf);
        strcat(head, "\r\n\r\n");
}

int makeResponse(char* response, int nr,const char* htmlFile)
{
	if(response==NULL || nr==0) return 0;

        int nread=0;
	FILE* fd=0;
	strcpy(response,"");
        fd = fopen(htmlFile,"r");
        if(fd==NULL)
        {       printf("\nError fopen ' %s '",htmlFile);
                return 0;
        }
	
	int i = 0;int ch;
	while((ch=fgetc(fd)) != EOF)
	{response[i] = ch;i++;if(i==nr-1) break;}
	response[i] = '\0';
	nread = strlen(response);

	printf("\n[length %i]\n%s",nread,response);

	fclose(fd);
	return nread;
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
        printf("\nGoodbye server'\n");
        bOut=true;
        for(int i=0;i<iConnect;i++)
        {
                pthread_cancel(thId[i]);
                thId[i]=0;
        }
        //pthread_mutex_destroy(&mutex);
        exit(0);
    }
}

