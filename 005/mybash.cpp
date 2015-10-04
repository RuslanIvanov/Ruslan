#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <list>
#include <iostream>
using namespace std;

struct listPath
{
	char buf[256];
};

pid_t pid_;

void out(int sig=0);
int  startProc(const char* path,const char* pName);
bool findPath(const char* pName, char *findedPath, char** env);
void printList(list<listPath>&List);
void parserPath(char* bufIn, list<listPath>& list);

int main(int argc,char* argv[], char** env)
{
	signal (SIGTERM, out);  
	signal (SIGINT, out);  
	printf("\n*********************************Run my bash****************************************\n");
	while(1)
	{	
		printf("\nEnter the name of the program: ");
		char bufName[256]="";
		scanf("%s",&bufName[0]);

		char bufPath[256]="";
		if(findPath(bufName,&bufPath[0],env)==false)
		{
			printf("\nNo find process ' %s '",&bufName[0]);
		}else {	startProc(bufPath,&bufName[0]); sleep(1);}

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
                { perror("execl"); exit(0); }
	     return 0;
	}

	if (pid==-1)
	{
		perror("fork");
		return -1;
	}

	if(pid>0)
        {
		printf("\nI'm the parent process, I started the process with the PID is ' %d' and name is ' %s ' ", pid, pName);
		return pid;
	}
}

bool findPath(const char* pName, char *findedPath, char** env)
{
	char bufPath[256]="";

	while(*env++)
	{
		char * p = strstr(*env,"PATH=");
		if(*env==p) 
		{
			strcpy(&bufPath[0],p+5);
			break;
		}
	}

	strcat(&bufPath[0],":.:");
	//printf("\npaths: %s\n",bufPath);
	list<listPath> lp;

	parserPath(bufPath,lp);

	if(lp.empty())
	{
		return false;
	}

	list<listPath>::iterator iter;
	bool isFind = false;
	for(iter=lp.begin();iter!=lp.end();iter++)
	{

		DIR *dfd;  
    		struct dirent *dp;
		dfd=opendir((*iter).buf);  
    		while( (dp=readdir(dfd)) != NULL )  
            	{	
			//printf("%s\n", dp->d_name);
			if(strcmp(dp->d_name,pName)==0)
			{
				strcat(findedPath,(*iter).buf);	
				strcat(findedPath,"/");	
				strcat(findedPath,pName);
				isFind=true;
				break;	
			} 
			if(isFind) break; 
		}

    		closedir(dfd);    		

	}	

	return isFind;
}

void parserPath(char* bufIn, list<listPath>& list)
{/*/usr/lib/lightdm/lightdm:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:./*/
	int i=0;int offset=0;
	while(bufIn[i]!='\0')
	{
		if(bufIn[i]==':')
		{
			bufIn[i]='\0';
			listPath lpath;
			strcpy(lpath.buf,&bufIn[0]+offset);
			list.push_back(lpath);
			offset = i+1;
		}
	
		i++;
	}

	printList(list);
}

void printList(list<listPath>&List)
{
	if(List.empty())
	{
		printf("\nList is empty!\n");
		return;
	}

	list<listPath>::iterator iter;

	for(iter=List.begin();iter!=List.end();iter++)
	{
		printf("\npath:{%s}",(*iter).buf);
	}

	printf("\n");
}
