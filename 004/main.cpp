#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <list>
using namespace std;

int getppid(int);
void getname(int,char*);
void listTree(int pidStart);

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
		printf("\nThe parents %d-{%s} is:\n",pid,name);

		listTree(pid);

		printf("\n*****************************************\n");

	}
	else
	{
	    listTree(0);
	}
	return 0;
}

int getppid(int pid)
{
	if(pid==0) {printf("Error: getppid: number pid!"); return 0;}

	char buf[256];

	sprintf(buf,"/proc/%d/status",pid);
	//printf("\nIm is finding parent in %s\n",buf);

	FILE* fd=0;
	fd = fopen(buf,"r");
	if(fd==NULL){printf("\nError fopen [%s]",buf); return 0;}

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
        if(fd==NULL){printf("\nError fopen [%s]",buf); return;}

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

void listTree(int pidStart)
{
	list<process> dirList;
	int ppid = -1;

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
		printf("\nList is empty!\n");
		return;
	}

	list<process>::iterator iter;

	for(iter=List.begin();iter!=List.end();iter++)
	{
		printf("\n%10d-{%s}",(*iter).pid,(*iter).name);
	}

	printf("\n");
}

