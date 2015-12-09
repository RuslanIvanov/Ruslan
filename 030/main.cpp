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
#include "driver/ioctl_kbuf.h"

int fd;
char filename[BUFSIZ];
void out(int sig=0);
char buf[BUFSIZ];
bool bOut = false;
void getStatictic();
void getPid(int);

int main(int argc,char* argv[], char** env)
{

	signal (SIGTERM, out);
	signal (SIGINT, out);

	strcpy(&filename[0],"/dev/chkbuf");

	printf("\nfile ' %s ', exit ' Ctrl+C '\n",&filename[0]);

	fd = open(&filename[0],O_RDWR); //O_RDONLY,O_WRONLY,O_RDWR 
	if(fd==-1) {printf("\nError open %s\n",filename); return 0;}


	getPid(fd,getpid());

	int countStr=0;
	while(bOut==false)
	{
	    sleep(1);
	    int rez=0;

	    sprintf(buf,"It is numper string %d\n", countStr++);

	    rez = write(fd,buf,strlen(buf));
	    if(rez==-1) {printf("Error write %s\n",filename); return 0;}
	    printf("\nwrited %d bytes",rez);

	    char tmp[/*BUFSIZ*/100];
	    rez = read(fd,tmp,100);

	    if(rez==-1) {printf("Error read %s\n",filename); return 0;}
	    printf("\nread %d bytes: \n",rez);
	    for(int i = 0;i<rez;i++)
		printf("%x",tmp[i]);
	    printf("\n");
	}

	close(fd);
	
	return 0;
}

void getStatictic(int fd)
{
	if(fd == 0) return;
}

void getPid(int fd ,int _pid)
{
	if(fd == 0) return;

	if(ioctl(fd,KBUF_IOCX_IO_PID,(int)&_pid)<0) // подумать ка получить инф о pid in US - двухстронний обмен
	{perror("ioctl: KBUF_IO_PIDS");}
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye chat(signal %d)\n",sig);
	bOut=true;
    }
}
