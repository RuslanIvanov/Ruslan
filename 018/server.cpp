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
#include <syslog.h>

#define MAX_CONNECT 20

int port=8080;
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

int makeResponseText(char* response, int nr,const char*);
int makeResponseImg(char* response, int nr,const char* imgFile);
void head(char *head,int n, int type );
int parserRequest(const char* str,char*,int n);

char logMsg[BUFSIZ]={'\0'};
int pidd;
//cmd -ldeamon
int main(int argc, char* argv[])
{
    signal (SIGTERM, out);
    signal (SIGINT, out);
    
    if(argc==3 && strcmp("-ldeamon",argv[2])==0)
    {

    int pid = fork();

    if (pid == -1)
    {
        perror("start daemon");
        return 0;
    }
    else if (pid==0) //child - deamon
    {
	pidd = getsid(pid);

	if(pidd)
	{
	    setsid();
	}else perror("getsid");

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

    }else { printf ("\nExit parent"); return 0;}// parent
    }//
    if(argc==2)
    {
	port = atoi(argv[1]);
    }
    printf("\nport %d\n",port);

    sprintf(logMsg,"START DEAMON (PID %d): port %d",pidd,port);
    syslog(0, logMsg, strlen(logMsg));

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

	printf("\nServer connected %d! Create socket %x\n",iConnect+1,sock);
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

    sprintf(logMsg,"STOP DEAMON (PID %d): port %d",pidd,port);
    syslog(0, logMsg, strlen(logMsg));
 
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
	    buf[bytes_read] = '\0';
	    printf("\n%s\n",buf);	

	    //разобрать заголовок
	    char nameRequest[BUFSIZ];
	    int rez  = parserRequest(buf,nameRequest,bytes_read);

	    char answer[BUFSIZ*40];
	    char response[BUFSIZ*40];
	    printf("\nsize response %ul",sizeof (response));
		
	    if( rez==1 )
	    {
		int len = makeResponseImg(response, sizeof (response),nameRequest);
		head(answer,len,rez);
		printf("\nanwer:\n%s\n",answer);
		int na = strlen(answer);
		sendto(pparam->sockTh, answer, na,0,( struct sockaddr *)(&pparam->addr),pparam->n);

		printf("\nnimg %d",len);
		int total = 0;
    		int n;

	        while(total < len)
    		{
        		n = sendto(pparam->sockTh, response+total, len-total,0,( struct sockaddr *)(&pparam->addr),pparam->n);
		        if(n == -1) { break; }
		        total += n;
			printf(" Send: %d",n);
    		}
	    }
            else
	    if( rez==0 )
	    {
		int len = makeResponseText(response, BUFSIZ,nameRequest);
		head(answer,len,rez);
		printf("\nanwer:\n%s\n",answer);
		strcat(answer,response);
		sendto(pparam->sockTh, answer, strlen(answer),0,( struct sockaddr *)(&pparam->addr),pparam->n);
	    } else 
		    {	int len = makeResponseText(response, BUFSIZ,"404_Not_Found.html");
			head(answer,len,rez);
                        strcat(answer,response);
			sendto(pparam->sockTh, answer, strlen(answer),0,( struct sockaddr *)(&pparam->addr),pparam->n);
	            }

	}
        shutdown(pparam->sockTh, 2);
        close(pparam->sockTh);
        printf("\nclose socket %d",pparam->sockTh);

        return 0;
}

int parserRequest(const char* str,char *name, int n)
{
	strcpy(name,"");
	char *pp = strstr((char*)str,"GET /index.html");
	if(pp) {strcpy(name,"index.html"); printf("\nfinded: index.html"); return 0;}	

	pp = strstr((char*)str,"GET / ");
	if(pp) {strcpy(name,"index.html"); return 0;}	

	pp = strstr((char*)str,"GET /Images/IMG_0599.JPG");
	if(pp)
	{
		strcpy(name,"Images/IMG_0599.JPG"); return 1;
		printf("\nfinded: IMG");
	}
	return -1;
}

void head(char *head,int n, int type)
{ 
	strcpy(head,"HTTP/1.1 200 OK\r\n");
        strcat(head, "Version: HTTP/1.1\r\n");
	if(type==0 || type == -1)
        	strcat(head, "Content-Type: text/html; charset=utf-8\r\n");
	if(type ==1 )
		strcat(head, "Content-Type: image/jpeg\r\n");
        strcat(head, "Content-Length: ");char buf[BUFSIZ]; sprintf(buf,"%i",n);
	strcat(head, buf);
        strcat(head, "\r\n\r\n");
}

int makeResponseImg(char* response, int nr,const char* imgFile)
{
	if(response==NULL || nr==0) return 0;

        int nread=0;
	FILE* fd=0;
	
        fd = fopen(imgFile,"rb");
        if(fd==NULL)
        {
		printf("\nError fopen ' %s '",imgFile);
		perror("");
                return 0;
        }
	
	int i = 0;
	while(!feof(fd))
	{
		//считываем элемент
		fread(response+i, sizeof(char), 1, fd);
		//printf("%x",response[i]);
		i++;
		if(i>=nr) break;
	}

	nread = i-1;

	printf("\n[length img %d]\n",nread);

	fclose(fd);
	return nread;
}

int makeResponseText(char* response, int nr,const char* htmlFile)
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

	sprintf(logMsg,"STOP DEAMON (kill %d)(PID %d): port %d",sig,pidd,port);
	syslog(0, logMsg, strlen(logMsg));

        exit(0);
    }
}

