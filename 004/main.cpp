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
	int ppid = -1;
	if(pidStart>0)
	{
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

	}else
	{
		DIR *pdir;
		struct dirent *ent;
		pdir = opendir("/proc/");
		if(pdir==NULL)
		{printf ("\nError opendir proc"); return ;}

		dirent *pdirent;
		//pdirent readdir(pdir);

		while(NULL != (pdirent = readdir(pdir)) )
        	{// как понять дирректория или нет?
			struct stat statbuf;
			lstat( pdirent->d_name, &statbuf ); // считываем информацию о файле в структуру
        		if (S_ISDIR( statbuf.st_mode ))
			{ // если это директория
			      if(strcmp( ".", pdirent->d_name ) == 0 ||
				 strcmp( "..",pdirent->d_name ) == 0 )
				{
				    continue; 
				}

				int pid = atoi(pdirent->d_name);
				if(pid==0) continue;

				 list<process> dirAllList;

	 			printf("\npid [%d] have a parents:",pid);
				while(getppid(pid)!=0) //убрать в ф_ию  listTree (pid), а то что выше вынести за пределы ф. listTree
                		{
		                        ppid = getppid(pid);
                		        pid = ppid;

		                        process pr;
                		        char name[50];
		                        getname(ppid,name);
                		        pr.pid=ppid;
		                        strcpy(pr.name,name);
                		        dirAllList.push_back(pr);

                		}
				printList(dirAllList);
			}
		}

		closedir(pdir);
	}
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

