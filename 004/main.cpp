#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getppid(int);
void listTree(int pidStart);

int main(int argc,char* argv[], char** env)
{
	if(argc>1)
	{	int pid = atoi(argv[1]);
		printf("\nThe parent %d is %d\n",pid,getppid(pid));
	}
	else 
	{
	    listTree(0);
	}
	return 0;
}

int getppid(int pid)
{
	if(pid==0) {printf("Error number pid!"); return 0;}

	char dir[10]="/proc/";
	char file[10]="/status";
	char buf[256];

	sprintf(buf,"%s%d%s",dir,pid,file);
	printf("\nIm is finding parent in %s\n",buf);

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
                    		printf("\nI'm find string: %s\n",pfind);
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
	
}
