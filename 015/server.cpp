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
bool getNameCli(const char* bufIn, char* pName,int nBuf);

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
    }else { printf("\nSet command string: 'port' 'name', Exit 'Ctrl+C'\n"); return 0; }

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
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

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
	if(errno  == ECONNABORTED && sock < 0)
	{
		printf("\nsocket %d disconnect",sock); 
		pthread_mutex_lock(&mutex);
		m_connect.erase(sock); 
    		pthread_mutex_unlock(&mutex);
		continue;
	}
	//int yes = 1;
        //if(setsockopt(listener, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) != -1){perror("SO_KEEPALIVE:"); }

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
	int bConnect = false;
	while(bOut==false)
        {
	    char buf[BUFSIZ]={'\0'};
            bytes_read = recv(pparam->sockTh, buf, BUFSIZ, 0);
            if(bytes_read <= 0) break;

 	    //printf("\n->'%s'",buf);
	    pthread_mutex_lock(&mutex);
		
	    if(bConnect==false)
	    {
	    	listConnect(buf,pparam->sockTh,m_connect);
		bConnect = true;
	    }
		
 	    int cmd = getCmd(buf);
	    if(cmd == 0)
	    {
		//printf("\nmsg from client (cmd %d)",cmd);
		listMsg(buf,v_msg);
	    }

	    if(cmd == 1) 
	    {
		//printf("\nsend to client names (cmd %d)",cmd);  
		map<int,string>:: iterator it;
		string tmp="";
		for (it = m_connect.begin(); it != m_connect.end(); it++)
  		{
			tmp+=(*it).second;tmp+="\n";
			//sendto(pparam->sockTh,  (*it).second.c_str(), strlen( (*it).second.c_str())+1, MSG_DONTWAIT,( struct sockaddr *)(&pparam->addr),pparam->n);// не отсылает все???
  		}
		sendto(pparam->sockTh,  tmp.c_str(), strlen( tmp.c_str())+1, MSG_DONTWAIT,( struct sockaddr *)(&pparam->addr),pparam->n);
	    }

	    if(cmd == 2)
	    {	//printf("\nsend to client messages (cmd %d)",cmd);
	    	if(v_msg.empty()==false)
	    	{
			string str="";
	    		vector<string>::reverse_iterator riter = v_msg.rbegin();
	    		for(int i=0;riter!=v_msg.rend();i++,++riter)	
             		{
				if(i>=10) break;
				str+=(*riter);str+="\n";
	    		}
			sendto(pparam->sockTh, str.c_str(), strlen(str.c_str())+1, MSG_DONTWAIT,( struct sockaddr *)(&pparam->addr),pparam->n);
	    	}
	     }
	     pthread_mutex_unlock(&mutex);
        }
    	shutdown(pparam->sockTh, 2);
        close(pparam->sockTh);
	printf("\nclose socket %d",pparam->sockTh);

	pthread_mutex_lock(&mutex);
	printf("\nDisconnect client ' %s '",m_connect[pparam->sockTh].c_str());
	m_connect.erase(pparam->sockTh);
	pthread_mutex_unlock(&mutex);
	return 0;
}

int getCmd(const char* bufIn)
{
	int i=0; 
	char tmp[BUFSIZ]= {'\0'};
	char  *p = strstr((char*)bufIn,"$C=");
	if(p)
	{ 
		p=p+3;
        	while(p[i]!='\0')
        	{
			tmp[i]=p[i];
                	if(tmp[i]==':')
                	{
                        tmp[i]='\0';
                        if(strcmp(tmp,"cln")==0) return 1;
			if(strcmp(tmp,"msg10")==0) return 2;
                	}
		
        	        i++;
        	}
		return -1;
	}else{
		p = strstr((char*)bufIn,"$M=");
		if(p){return 0;}
	     }

	return -1;
}

void listConnect(const char* bufIn, int fd, map<int,string>& mc)
{
        int i=0; 
	char tmp[BUFSIZ]= {'\0'};
	char  *p = strstr((char*)&bufIn[0],"$N=");
	if(p==NULL) return ;
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

bool getNameCli(const char* bufIn, char* pName,int nBuf)
{
        int i=0; 
	
	char  *p = strstr((char*)&bufIn[0],"$N=");
	if(p==NULL) return false;
	if(pName==NULL) return false;

	p=p+3;
        while(p[i]!='\0')
        {
		if(i>=nBuf) return false;
		pName[i]=p[i];
                if(pName[i]==':')
                {
                        pName[i]='\0';             
			return true;
                }
		
                i++;
        }
	return false;
}

void listMsg(const char* bufIn, vector<string>& vc)
{
	char  *p = strstr((char*)&bufIn[0],"$M=");
	if(p==NULL) return ;
	p=p+3;
	string strMsg(p);

	char bufName[BUFSIZ]={'\0'};
	string strName="";
	if(getNameCli(bufIn, bufName,BUFSIZ))
		strName.append(bufName);


        vc.push_back(strName+": "+strMsg);
	printf("\nserver chat: %s: %s",strName.c_str(),strMsg.c_str());
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye server'\n");
	bOut=true;
	for(int i=0;i<iConnect;i++)
	{
	
		//close(thId[i]);	
		pthread_cancel(thId[i]);
		thId[i]=0;
	}
	pthread_mutex_destroy(&mutex);
	exit(0);
    }
}
