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

void getname(char* buf,const char * pFind,char* pn);
void allTasks(int);

int main(int argc,char* argv[], char** env)
{
	if(argc==2)
	{	int pid = atoi(argv[1]);
		if(pid<=0)
			{printf("\nInvalid pid, plesae enter pid >0! Exit...\n"); return 0;}

		printf("\n**********************************************************\n");

		allTasks(pid);		

		printf("\n\n******************************************************\n");
	}else  {printf("\nError command string! Enter: 'PID'.\n");}

	printf("\n\nExit...\n");
	return 0;
}

void getname(char* path,const char * pFind,char* pn)
{

 	FILE* fd=0;
        fd = fopen(path,"r");
        if(fd==NULL){printf("\nError fopen [%s]",path); strcpy(pn,""); return;}

        int ch = 0;
        int i=0;
        char bufString[BUFSIZ];

        while((ch=fgetc(fd)) != EOF)
        {
		bufString[i] = ch;
                if((char)ch == 0x0a)
                {

                           bufString[i] = '\0';
                           i=0;

                           char * pname = strstr(bufString,pFind);
                           if(pname!=NULL)
                           {
                                char buf[BUFSIZ]; char name[100];
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

void allTasks(int pid)
{
	DIR *pdir;
	struct dirent *ent;
	char bufPathTask[BUFSIZ];
	sprintf(bufPathTask,"/proc/%d/task",pid);
	pdir = opendir(bufPathTask);
	if(pdir==NULL)
	{
		printf ("\nError opendir %s",bufPathTask); 
		printf ("\nSuch a process %d does not exist",pid);	
		return ;
	}

	dirent *pdirent;

	char pathProcess[BUFSIZ];
	sprintf(pathProcess,"/proc/%d/status",pid);
	char name[50]; getname(pathProcess,"Name:",name);
	printf("\nThe process '%s' [pid %d] has a threads:",name,pid);

	while(NULL != (pdirent = readdir(pdir)) )
       	{
		struct stat statbuf;
		lstat( pdirent->d_name, &statbuf ); // считываем информацию о файле в структуру
        	if (S_ISDIR( statbuf.st_mode ))
		{ // если это директория
		      if(strcmp( ".", pdirent->d_name ) == 0 ||
			 strcmp( "..",pdirent->d_name ) == 0 )
			{continue;}

			int tid = atoi(pdirent->d_name);
			if(tid==0) continue;

			char bufPathThread[BUFSIZ];char nameThread[BUFSIZ];
			sprintf(bufPathThread,"%s/%d/status",bufPathTask,tid);
			getname(bufPathThread,"Name:",&nameThread[0]);
			printf("\n\ttid %d {%s}",tid,nameThread);

		}
	}
	closedir(pdir);
}


