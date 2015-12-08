#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFFER_SIZE 100

int fd;
char filename[BUFSIZ];
void out(int sig=0);
char buf[BUFSIZ]="first string";
bool bOut = false;

int main(int argc,char* argv[], char** env)
{

	signal (SIGTERM, out);
	signal (SIGINT, out);

	strcpy(&filename[0],"/dev/chkbuf");

	printf("\nfile ' %s ', exit ' Ctrl+C '\n",&filename[0]);

	fd = open(&filename[0],O_RDWR); //O_RDONLY,O_WRONLY,O_RDWR 
	if(fd==-1) {printf("\nError open %s\n",filename); return 0;}

//	while(bOut==false)
	{
	    sleep(1);
	    int rez=0;
	    rez = write(fd,buf,strlen(buf));
	    if(rez==-1) {printf("Error write %s\n",filename); return 0;}
	    printf("\nwrited %d bytes",rez);

	    char tmp[20];
	    rez = read(fd,tmp,strlen(tmp));

	    if(rez==-1) {printf("Error read %s\n",filename); return 0;}
	    printf("\nread %d bytes: \n",rez);
	    for(int i = 0;i<rez;i++)
		printf("%c",tmp[i]);
	    printf("\n");
	}

	close(fd);
	
	return 0;
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye chat(signal %d)\n",sig);
	bOut=true;
    }
}
