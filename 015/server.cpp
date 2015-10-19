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

using namespace std;

#define MAX_CONNECT 100

unsigned int port;
void out(int sig=0);
bool bOut = false;
char nameServ[BUFSIZ];
void * funcThread(void*);
vector <string> v_msg;
vector <string> v_connect;

struct Param
{
      struct sockaddr_in addr;
      int sockTh;
      int n;
};

Param param[MAX_CONNECT];
pthread_t thId[MAX_CONNECT];
int iConnect;
void listConnect(const char* bufIn, vector<string>& vc);
void listMsg(/*const*/ char* bufIn, vector<string>& vc);
int getCmd(const char* bufIn);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[])
{

    signal (SIGTERM, out);
    signal (SIGINT, out);

    struct sockaddr_in addr;
    struct sockaddr_in addrCli[MAX_CONNECT];
    memset(&addr,0,sizeof(struct sockaddr_in));
    memset(&addrCli,0,sizeof(struct sockaddr_in)*MAX_CONNECT);

    if(argc>1 && argc<=3)
    {
	port = atoi(argv[1]);
	if(port<=0){printf("\nError number port"); return 0;}

	addr.sin_family = AF_INET;
    	addr.sin_port = htons(port); 
	
	strcpy(nameServ,argv[2]); 

	 pthread_mutex_init(&mutex, NULL);
    }else { printf("\nSet command string: 'port' 'name'\n"); return 0; }

    int sock, listener;
   
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
    addr.sin_addr.s_addr = htonl(INADDR_ANY);//htonl(INADDR_LOOPBACK);//

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

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
	printf("\nServer connected %d!",iConnect+1);
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
	pthread_mutex_destroy(&mutex);

    return 0;
}

void * funcThread(void* pVoid)
{
	printf("\nSTART TASK");
	int bytes_read=0;
	Param *pparam = (Param*)pVoid;
	
	while(bOut==false)
        {
	    char buf[BUFSIZ]={'\0'};
            bytes_read = recv(pparam->sockTh, buf, BUFSIZ, 0);
            if(bytes_read <= 0) continue;

	    pthread_mutex_lock(&mutex);
	    listConnect(buf,v_connect);
		
 	    int cmd = getCmd(buf);
	    if(cmd == 0)
	    {
		listMsg(buf,v_msg);
	    }

	    if(cmd == 1) 
	    {
		for(int i=0;i<v_connect.size();i++)
	    	{    		
			send(pparam->sockTh, v_connect[i].c_str(), strlen(v_connect[i].c_str())+1, 0);
	    	}
	    }

	    if(cmd == 2)
	    {	
	    	//выдавать послед. 10 из списка,	
	    	printf("\nserver read[%d]: %s",bytes_read,buf);
	    	if(v_msg.empty()==false)
	    	{
	    		vector<string>::reverse_iterator riter = v_msg.rbegin();
	    		for(int i=0;riter!=v_msg.rend() && i<10;i++,++riter)	
             		{
				send(pparam->sockTh, (*riter).c_str(), strlen((*riter).c_str())+1, 0);
	    		}
	    	}
	     }
	     pthread_mutex_unlock(&mutex);
        }
    
        close(pparam->sockTh);
	return 0;
}

int getCmd(const char* bufIn)
{
	int i=0; 
	char tmp[BUFSIZ]= {'\0'};
	char  *p = strstr(tmp,"$C=");
	p=p+3;
        while(p[i]!='\0')
        {
		tmp[i]=p[i];
                if(tmp[i]==':')
                {
                        tmp[i]='\0';
                        if(strcmp(tmp,"cln")) return 1;
			if(strcmp(tmp,"msg10")) return 2;
                }
		
                i++;
        }

	return 0;
}

void listConnect(const char* bufIn, vector<string>& vc)
{
        int i=0; 
	char tmp[BUFSIZ]= {'\0'};
	char  *p = strstr(tmp,"$N=");
	p=p+3;
        while(p[i]!='\0')
        {
		tmp[i]=p[i];
                if(tmp[i]==':')
                {
                        tmp[i]='\0';
                        string str(tmp);

			bool b=false;
			for(int i=0;i<vc.size();i++)
			{
				if(vc[i]==str){b=true; break;}
			}
			if(b==false)
			{vc.push_back(str); printf("\nadd: %s",str.c_str());}
			break;
                }
		
                i++;
        }
}

void listMsg(/*const*/ char* bufIn, vector<string>& vc)
{
	char  *p = strstr(&bufIn[0],"$M=");
	p=p+3;
	string str(p);
        vc.push_back(str);
	printf("\nserver chat: %s",bufIn);
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye server'\n");
	bOut=true;
	for(int i=0;i<iConnect;i++)
	{
		close(thId[i]);	
		thId[i]=0;
	}
	pthread_mutex_destroy(&mutex);
	exit(0);
    }
}
