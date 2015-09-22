#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char* argv[], char** env)
{
	char dir[10]="/proc/";
	char file[10]="/status";
	char buf[50];
	char bufString[256];
	FILE* fd=0;
	if(argc>1)
	{
		sprintf(buf,"%s%d%s",dir,atoi(argv[1]),file);
		printf("\nfind parent pid in: %s\n",buf);
	   	fd = fopen(buf,"r");
		if(fd==NULL){printf("\nError fopen [%s]",buf); return 0;}
		int ch = 0; int i=0;
		while((ch=fgetc(fd)) != EOF)
		{//sscanf!!!
		    buf[i] = (char)ch;
		    if(ch == '\0')
		    {
			    printf("%c",(char)ch);
			    i=0;
			    char * p = strstr(buf,"PPid:");
            		    if(buf==p) 
            		    {
                    		printf("\nis find ppid: %s\n",p);
				unsigned int ppid=0;
				sscanf(ppid,"%d",p);
				printf("\npppid = %");
			    }

            	    }

			        
		}
		i++;
		    
		}
		
	}

	if(fd!=0)
	fclose(fd);
    	return 0;
}
