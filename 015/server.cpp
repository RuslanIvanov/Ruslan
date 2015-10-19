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
void listMsg(const char* bufIn, vector<string>& vc);

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
	printf("\nServer connected!");
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
	printf("\nSTART TASK");
	int bytes_read=0;
	Param *pparam = (Param*)pVoid;
	
	while(bOut==false)
        {
	    char buf[BUFSIZ]={'\0'};
            bytes_read = recv(pparam->sockTh, buf, BUFSIZ, 0);
            if(bytes_read <= 0) continue;
//mutex
	    listConnect(buf,v_connect);
	    listMsg(buf,v_msg);
//mutex
	
	    //выдавать послед. 10 из списка,	
	    printf("\nserver read[%d]: %s",bytes_read,buf);
	    if(!v_msg.empty())
	    {
	    	vector<string>::reverse_iterator riter = v_msg.rbegin();
	    	for(int i=0;riter!=v_msg.rend() || i<10;i++,++riter)	
             	{
			char tmp[BUFSIZ];
			//strcpy(tmp,(*riter).c_str());
			sprintf(tmp,"%s\n",(*riter).c_str());
			send(pparam->sockTh, tmp, strlen(tmp)+1, 0);
	    	}
	    }
        }
    
        close(pparam->sockTh);
	return 0;
}


void listConnect(const char* bufIn, vector<string>& vc)
{
        int i=0; char tmp[BUFSIZ];
	strcpy(tmp,bufIn);
        while(tmp[i]!='\0')
        {
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
                        vc.push_back(str);
			break;
                }

                i++;
        }
}

void listMsg(const char* bufIn, vector<string>& vc)
{
	string str(bufIn);
        vc.push_back(str);
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
	exit(0);
    }
}
