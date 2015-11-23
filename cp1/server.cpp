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
#include <errno.h>

#define  MAX_CONNECT 1

int  port;
void out (int sig=0);
void usr1(int sig);
void usr2(int sig);
bool bOut = false;
int  parserRequest(char* str,int n);
char logMsg[BUFSIZ]={'\0'};
int pidd;

unsigned int countUserSig[2];
bool bOnUsr[2]={true,true};

struct statCmd
{
	unsigned int countOn;
	unsigned int countOff;
};

statCmd sCmd[2];

int main(int argc, char* argv[])
{
    	signal (SIGTERM,out);
    	//signal (SIGINT, out);
    	signal (SIGUSR1,usr1);
	signal (SIGUSR2,usr2);

    	if(argc != 2 )
    	{
		printf("\nError param in command string: 'port' \n");
	 	return 0;
    	}

	port = atoi(argv[1]);
	printf("\nstart deamon...\n");
    	int pid = fork();

    	if (pid == -1)
    	{perror("start daemon");return 0;}
	else if (!pid) //child - deamon
    		{
			 pidd = getsid(pid);// возвращает идентификатор (ID) сессии, вызвавшего процесса

			 if(pidd!=-1)
			 {
				if(setsid()==-1)
				{
					sprintf(logMsg,"DEAMON setsid error (errno %d)",errno);
                                	syslog(0, logMsg, strlen(logMsg));
				}

			 }

			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

    		}else { printf ("\nExit parent\n"); return 0;}// parent

    if(port==0)
    {
	sprintf(logMsg,"Error params in command string by deamon");
       	syslog(0, logMsg, strlen(logMsg));
	return 0;
    }

    pidd = getpid();
    sprintf(logMsg,"START DEAMON (PID %d): port %d",pidd,port);
    syslog(0, logMsg, strlen(logMsg));   

    int sock, listener;
    char buf[BUFSIZ];
    int bytes_read;
    struct sockaddr_in addr;
    struct sockaddr_in addrCli;
    memset(&addr,0,sizeof(struct sockaddr_in));
    memset(&addrCli,0,sizeof(struct sockaddr_in));

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
         strerror_r(errno,logMsg, BUFSIZ);
         syslog(0, logMsg, strlen(logMsg));

         return 0;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int optval = 1;
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)==-1)
    {
	  strerror_r(errno,logMsg, BUFSIZ);
          syslog(0, logMsg, strlen(logMsg));
    }

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
	strerror_r(errno,logMsg, BUFSIZ);
	syslog(0, logMsg, strlen(logMsg));

        return 0;
    }

    listen(listener, MAX_CONNECT);

    int nAddrCli=0;
    sock = accept(listener, (struct sockaddr*)&addrCli, (socklen_t *)&nAddrCli);
    if(sock < 0)
    {
	strerror_r(errno,logMsg, BUFSIZ);
        syslog(0, logMsg, strlen(logMsg));
        return 0;
    }

    while(bOut==false)
    {
    	int bytes_read = recv(sock, buf, BUFSIZ, 0);
        if(bytes_read <= 0) 
	{
		strerror_r(errno,logMsg, BUFSIZ);
		syslog(0, logMsg, strlen(logMsg));
 
	}

	buf[bytes_read] = '\0';

	//разобрать заголовок
	int rez  = parserRequest(buf,bytes_read);

	if(rez == 1) { bOnUsr[0] = true; }
	if(rez == 2) { bOnUsr[1] = true; }

	if(rez == 3) { bOnUsr[0] = false; }
	if(rez == 4) { bOnUsr[1] = false; }

	char response[BUFSIZ];

	if(rez==5)
		sprintf(response," user1 (on = %d, off = %d), user2 (on = %d, off = %d). Count: SIGUSR1 %d, SIGUSER2 %d ",sCmd[0].countOn,sCmd[0].countOff,sCmd[1].countOn,sCmd[1].countOff,countUserSig[0],countUserSig[1]);
	else if(rez==0)
	    sprintf(response," error command ");
	else sprintf(response," command is success ");

	int len = strlen(response);
	int  n = sendto(sock, response, len,0,( struct sockaddr *)(&addrCli),sizeof(struct sockaddr));
        if(n == -1)
	{
	    strerror_r(errno,logMsg, BUFSIZ);
	    syslog(0, logMsg, strlen(logMsg));

	    break;
	}

   }

    shutdown(sock, 2);
    close(sock);

    sprintf(logMsg,"STOP DEAMON (PID %d): port %d",pidd,port);
    syslog(0, logMsg, strlen(logMsg));

    return 0;
}

int parserRequest(char* str, int n)
{//0 and > 5 is error, user command for SIGUSER1,2: "cmd:1" -  on user1, "cmd:2" - on user2, "cmd:3" - off user1, "cmd:4" - off user2, "cmd:5" - get statistic

	int cmd = 0;
	char * p = strstr(str,"cmd:");
        if(str==p)
        {
		sprintf(logMsg,"DEAMON: cmd: %s",p+4);
		syslog(0, logMsg, strlen(logMsg));
		cmd = atoi(p+4);

		if( cmd>5 || cmd == 0 )
		{
		    sprintf(logMsg,"DEAMON: cmd: error");
    		    syslog(0, logMsg, strlen(logMsg));
		    cmd = 0;
		}else
		{
		    if(cmd==1)
		    {
			sCmd[0].countOn++;
		    }
		    if(cmd==3)
		    {
			sCmd[0].countOff++;
		    }
		    if(cmd==2)
		    {
			sCmd[1].countOn++;
		    }
		    if(cmd==4)
		    {
			sCmd[1].countOff++;
		    }
		}
	}

	return cmd;
}

void usr1(int sig)
{
	if(sig==SIGUSR1 && bOnUsr[0])
    	{
		countUserSig[0]++;

		sprintf(logMsg,"DEAMON: SIGUSR1 %d",countUserSig[0]);
    		syslog(0, logMsg, strlen(logMsg));
	}
}

void usr2(int sig)
{
	if(sig==SIGUSR2 && bOnUsr[1])
    	{
		countUserSig[1]++;

		sprintf(logMsg,"DEAMON: SIGUSR2 %d",countUserSig[1]);
    		syslog(0, logMsg, strlen(logMsg));
	}
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
        bOut=true;
	sprintf(logMsg,"STOP DEAMON (kill %d)(PID %d): port %d",sig,pidd,port);
	syslog(0, logMsg, strlen(logMsg));

        exit(0);
    }
}

