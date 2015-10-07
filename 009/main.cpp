#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void invert(char*);

int main(int argc,char* argv[], char** env)
{

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

		char c;int i=0;
		while ((c = fgetc(fp)) != EOF) 
		{
	    		buf[i] = c;
		    	if(buf[i]=='\n')
	    		{
				buf[i]='\0';
				invert(buf);
				i=0;

	    		}else i++;
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
