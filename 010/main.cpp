#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned int countStr(const char*,const char*);
char filename[BUFSIZ]="countstr.tmp";
char findstring[BUFSIZ]="";
int main(int argc,char* argv[], char** env)
{
	if(argc>1&&argc<=3)
	{	
		
		strcpy(findstring,argv[1]);
		if(argc == 3)
		{
			if(strcmp("",argv[2])!=0 || strcmp("main.cpp",argv[2])!=0)
			strcpy(filename,argv[2]);
		}
		
		char buf[BUFSIZ];
		FILE *fp[]={NULL,NULL,NULL};

		fp[0]=fopen("/dev/stdin", "r");
		fp[1]=fopen("/dev/stdout", "a");
		fp[2]=fopen(filename, "w");
	
		unsigned int count=0;
		while (fgets(buf,BUFSIZ,fp[0]) != NULL) 
        	{
			printf("%s",buf);
			count+=countStr(buf,findstring);
		}
	
		fprintf(fp[2],"\n#' %s ' [%d]\n",findstring,count);
	
		fp[1]=fopen("/dev/stdout", "a");
		fputs(buf,fp[1]);

		for(unsigned char i=0;i<3;i++) fclose(fp[i]);

	}
	else {printf("\nError command string! Enter: 'search string' 'filename'.\n");}
	printf("\n\nExit...\n");
	return 0;
}

unsigned int countStr(const char* buf,const char* fstr)
{	
	unsigned int lenfind = strlen(fstr);
	unsigned int lenbuf = strlen(buf);
	unsigned int i=0;
	do
	{
		unsigned int len = strspn(&buf[i],fstr);
	
		if(lenfind == len)//стороки равны по длинне
		{
			return 1;
		}
		i++;
	}
	while(i<lenbuf);

	return 0;
}
