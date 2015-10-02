#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>
#include <list>
using namespace std;

int getppid(int);
void getname(int,char*);
void listTree2(int);
void allProcesses();

struct process
{
	char name[50];
	int pid;
};

void printList(list<process>&List);

int main(int argc,char* argv[], char** env)
{
	if(argc>1)
	{	int pid = atoi(argv[1]);

		printf("\n*****************************************\n");
		char name[50];
		getname(pid,name);
		printf("\nThe parents %d-{%s} is: ",pid,name);
		listTree2(pid);
		printf("\n*****************************************\n");
	}
	else
	{
	     allProcesses();
	}
	return 0;
}

int getppid(int pid)
{
	if(pid==0) {printf("Error: getppid: number pid!"); return 0;}

	char buf[256];

	sprintf(buf,"/proc/%d/status",pid);

	FILE* fd=0;
	fd = fopen(buf,"r");
	if(fd==NULL)
	{
		printf("\nError fopen [%s]",buf);
		printf("\nThe process with pid %d does not exist ",pid);
		return 0;
	}

	int ch = 0;
	int i=0;
	int ppid=0;
	char bufString[256];
	while((ch=fgetc(fd)) != EOF)
	{
//		printf("%c",(char)ch);
		bufString[i] = ch;
		if((char)ch == 0x0a)
		{

			    bufString[i] = '\0';
			    i=0;
			    char * pfind = strstr(bufString,"PPid:");

            		    if(pfind!=NULL)
            		    {
				char buf[256];
				sscanf(bufString,"%s%d",&buf[0],&ppid);
				break;
			    }
     		 }
		 i++;
	}

	if(fd!=0)
	fclose(fd);

	return ppid;
}

void getname(int pid,char* pn)
{

	char buf[256];
        sprintf(buf,"/proc/%d/status",pid);

 	FILE* fd=0;
        fd = fopen(buf,"r");
        if(fd==NULL){printf("\nError fopen [%s]",buf); strcpy(pn,""); return;}

        int ch = 0;
        int i=0;
        char bufString[256];

        while((ch=fgetc(fd)) != EOF)
        {
		bufString[i] = ch;
                if((char)ch == 0x0a)
                {

                           bufString[i] = '\0';
                           i=0;

                           char * pname = strstr(bufString,"Name:");
                           if(pname!=NULL)
                           {
                                char buf[256]; char name[100];
                                sscanf(bufString,"%s%s",&buf[0],&name[0]);
                                strcpy(pn,name);
				break;
                           }
		}
		i++;
	}

	if(fd!=0)
        fclose(fd);
}

void allProcesses()
{
	DIR *pdir;
	struct dirent *ent;
	pdir = opendir("/proc/");
	if(pdir==NULL)
	{printf ("\nError opendir proc"); return ;}

	dirent *pdirent;

	while(NULL != (pdirent = readdir(pdir)) )
       	{
		struct stat statbuf;
		lstat( pdirent->d_name, &statbuf ); // считываем информацию о файле в структуру
        	if (S_ISDIR( statbuf.st_mode ))
		{ // если это директория
		      if(strcmp( ".", pdirent->d_name ) == 0 ||
			 strcmp( "..",pdirent->d_name ) == 0 )
			{continue;}

			int pid = atoi(pdirent->d_name);
			if(pid==0) continue;

			char name[50]; getname(pid,name);
			printf("\nThe process '%s' [pid %d] has a parent:",name,pid);
			listTree2(pid);

		}
	}
	closedir(pdir);
}

void listTree2(int pidStart)
{
	int ppid = -1;
	list<process> dirList;

	while(getppid(pidStart)!=0)
	{
			ppid = getppid(pidStart);
			pidStart = ppid;

			process pr;
			char name[50];
			getname(ppid,name);
			pr.pid=ppid;
			strcpy(pr.name,name);
			dirList.push_back(pr);
	}

	printList(dirList);
}

void printList(list<process>&List)
{
	if(List.empty())
	{
		//printf("\nList is empty!\n");
		printf("\nNo parent!");
		return;
	}

	list<process>::iterator iter;

	for(iter=List.begin();iter!=List.end();iter++)
	{
		printf("\n%10d-{%s}",(*iter).pid,(*iter).name);
	}

	printf("\n");
}

