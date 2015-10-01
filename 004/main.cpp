#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <list>
using namespace std;

int getppid(int);
void listTree(int pidStart);
void printList(list<int>&List);

int main(int argc,char* argv[], char** env)
{
	if(argc>1)
	{	int pid = atoi(argv[1]);

		printf("\n*****************************************\n");
		printf("\nThe parent %d is %d\n",pid,getppid(pid));
		printf("\n*****************************************\n");

		listTree(pid);
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

	char dir[10]="/proc/";
	char file[10]="/status";
	char buf[256];

	sprintf(buf,"%s%d%s",dir,pid,file);
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
//			    printf("\npfind=%p [%s] [%s]",pfind,pfind,bufString);

            		    if(pfind!=NULL)
            		    {	char buf[256];
                    		//yprintf("\nI'm find string: %s\n",pfind);
				sscanf(bufString,"%s%d",&buf[0],&ppid);
				//printf("\npppid = %d",ppid);
				break;
			    }

     		 }
		i++;
	}

	if(fd!=0)
	fclose(fd);

	return ppid;
}

void listTree(int pidStart)
{
	list<int> dirList;
	int ppid = -1;

	while(getppid(pidStart)!=0)
	{
		ppid = getppid(pidStart);
		pidStart = ppid;
		dirList.push_back(ppid);
	}

	printList(dirList);
}


void printList(list<int>&List)
{
	if(List.empty())
	{
		printf("\nList is empty!\n");
		return;
	}

	list<int>::iterator iter;

	for(iter=List.begin();iter!=List.end();iter++)
	{
		printf("\t-- %d\n",*iter);
	}
}

