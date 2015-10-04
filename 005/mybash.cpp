#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <list>
#include <iostream>

void out(int sig=0);
int startProc(const char* path,const char* pName);
bool findPath(const char* pName, char *findedPath, char** env);
pid_t pid_imit;

int main(int argc,char* argv[], char** env)
{
	signal (SIGTERM, out);  
	signal (SIGINT, out);  

	while(1)
	{	
		printf("\nEnter: ");
		char bufName[256]="";
		scanf("%s",&bufName[0]);
		
		char bufPath[256]="";
		if(findPath(bufName,&bufPath[0],env)==false)
		{
			printf("\nNo find process ' %s '",&bufName[0]);
		}else {	startProc(bufPath,&bufName[0]); }
	
	}
	return 0;
}

void out(int sig)
{
	if(sig==SIGTERM||sig == SIGINT)
	{
		printf("\nGoodbye (signal %d)\n",sig);
	}
	exit(0);
}


int startProc(const char* path,const char* pName)
{// ф. запускa процесса: -1 - не запустился, 0 - запустился

        int pid = 0;
        pid = fork();

        if(!pid)// это дочерний процесс, замещаем его процессом pName
        {
	     printf("\nI was a child process!");
             if(execl(path,pName,NULL)==-1) 
                 return -1;
	 	return 0;
	}

	if (pid==-1)
	{
		perror("fork");
		return -1;
	}

	if(pid>0)
        {	
		printf("\nI'm the parent process, my 'pid' is %d", pid);
		return pid;
	}
}

bool findPath(const char* pName, char *findedPath, char** env)
{
	return true;
}
