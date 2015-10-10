#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale>
#include <iostream>

void invert(char*);

int main(int argc,char* argv[], char** env)
{

	setlocale(LC_ALL,"Russian" );
	printf("\nВыполнение...");
	if(argc>1)
	{
		FILE *fp;
		char temp[BUFSIZ];
		strcpy(temp,argv[1]);
		printf("\nargc %d\n",argc);
		for(int i=2;i<argc;i++)
		{
			strcat(&temp[0]," ");
			strcat(&temp[0],argv[i]);
		}

		printf("\ncmd: [%s]",temp);

		fp=popen(temp, "r");
		char buf[BUFSIZ];

		while (fgets(buf,BUFSIZ,fp) != NULL) 
		{
			invert(buf);
		}

		pclose(fp);
	}
	printf("\n\nExit...\n");
	return 0;
}

void invert(char* buf)
{
	int len = strlen(buf);
	for(int ii=0;ii<len/2;ii++)
	{
	    char temp = buf[ii];
	    buf[ii] = buf[len-ii-1];
	    buf[len-ii-1] = temp;
	}
	printf("\n%s",buf);
}
