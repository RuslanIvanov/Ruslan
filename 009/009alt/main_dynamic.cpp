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

		fpos_t pos;//тек. позиция в ф.
		fgetpos (fp,&pos);

		int count=1;
		while (fgetc(fp) != EOF) 
                {
			count++;
		}
		count++; printf("\ncount %d",count);
		char *pstr = new char[count];
		pstr[count-1]='\0';

		fsetpos (fp,&pos);//вернить позицию 

		char c=0;int i=0;
		while ((c = fgetc(fp)) != EOF)
		{
	    		pstr[i] = c;
	    		i++;
		}

		printf("\nold:\n%s",pstr);
                invert(pstr);

		delete [] pstr;
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
	printf("\ninvert:\n%s",buf);
}
