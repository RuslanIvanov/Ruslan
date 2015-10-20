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

unsigned int port;
void out(int sig=0);
bool bOut = false;
char nameServ[BUFSIZ];
void * funcThread(void*);
vector <string> v_msg;
map<int,string> m_connect;
//vector <string> v_connect;

struct Param
{
      struct sockaddr_in addr;
      int sockTh;
      int n;
};

Param param[MAX_CONNECT];
pthread_t thId[MAX_CONNECT];
int iConnect;
void listConnect(const char* bufIn, int ,map<int,string>&);
void listMsg(const char* bufIn, vector<string>& vc);
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
	if(sock == ECONNABORTED)//не работеат
	{
		printf("\nsocket %d disconnect",sock); 
		pthread_mutex_lock(&mutex);
		m_connect.erase(sock); 
    		pthread_mutex_unlock(&mutex);
		continue;
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
	pthread_mutex_destroy(&mutex);

    return 0;
}

void * funcThread(void* pVoid)
{
	int bytes_read=0;
	Param *pparam = (Param*)pVoid;
	
	while(bOut==false)
        {
	    char buf[BUFSIZ]={'\0'};
	    printf("\nwait server sock '%x' ",pparam->sockTh);
            bytes_read = recv(pparam->sockTh, buf, BUFSIZ, 0);
	    printf("\nbytes read %d",bytes_read);
            if(bytes_read <= 0) break;
 	    printf("\n->'%s'",buf);
	    pthread_mutex_lock(&mutex);
	    listConnect(buf,pparam->sockTh,m_connect);
		
 	    int cmd = getCmd(buf);
	    if(cmd == 0)
	    {
		printf("\ncmd %d",cmd);
		listMsg(buf,v_msg);
	    }

	    if(cmd == 1) 
	    {
		printf("\ncmd %d",cmd); printf("send to client ...");  
		map<int,string>:: iterator it;
		string tmp;
		for (/*auto*/it = m_connect.begin(); it != m_connect.end(); ++it)
  		{
     			printf("\nfd %d name: %s",(*it).first, (*it).second.c_str());
			sendto(pparam->sockTh,  (*it).second.c_str(), strlen( (*it).second.c_str())+1, O_NONBLOCK,( struct sockaddr *)(&pparam->addr),pparam->n);// не отсылает все
  		}
	    }

	    if(cmd == 2)
	    {	
		//printf("\ncmd %d",cmd);
	    	//выдавать послед. 10 из списка,	
	    	//printf("\nserver read[%d]: %s",bytes_read,buf);
	    	if(v_msg.empty()==false)
	    	{
	    		vector<string>::reverse_iterator riter = v_msg.rbegin();
	    		for(int i=0;riter!=v_msg.rend() && i<10;i++,++riter)	
             		{
				sendto(pparam->sockTh, (*riter).c_str(), strlen((*riter).c_str())+1, O_NONBLOCK,( struct sockaddr *)(&pparam->addr),pparam->n);
				printf("\nmsg: %s",(*riter).c_str());
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
	char  *p = strstr((char*)bufIn,"$C=");
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

void listConnect(const char* bufIn, int fd, map<int,string>& mc)
{
        int i=0; 
	char tmp[BUFSIZ]= {'\0'};
	char  *p = strstr((char*)&bufIn[0],"$N=");
	p=p+3;
        while(p[i]!='\0')
        {
		tmp[i]=p[i];
                if(tmp[i]==':')
                {
                        tmp[i]='\0';
                        string str(tmp);
			printf("\nname connect '%s' is add",tmp);
			mc.insert (pair<int,string>(fd,str));
			return;
                }
		
                i++;
        }
}

void listMsg(const char* bufIn, vector<string>& vc)
{
	char  *p = strstr((char*)&bufIn[0],"$M=");
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
