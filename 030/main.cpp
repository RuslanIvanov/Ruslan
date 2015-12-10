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
#include <sys/ioctl.h>
#include "driver/ioctl_kbuf.h"

int  fd;
char filename[BUFSIZ];
void out(int sig=0);
char buf[BUFSIZ];
bool bOut = false;
void getStatictic(int);
void getPid(int,int);

PID_INFO pidInfo;
STATISTIC_RW statistic;

int main(int argc,char* argv[], char** env)
{

	signal (SIGTERM, out);
	signal (SIGINT, out);

	strcpy(&filename[0],"/dev/chkbuf");

	printf("\nfile ' %s ', exit ' Ctrl+C '\n",&filename[0]);

	fd = open(&filename[0],O_RDWR); //O_RDONLY,O_WRONLY,O_RDWR 
	if(fd==-1) {printf("\nError open %s\n",filename); return 0;}

	int countStr=0;
	while(bOut==false)
	{
	    sleep(1);
	    int rez=0;

	    //sprintf(buf,"It is numper string %d\n", countStr++);
	    //rez = write(fd,buf,strlen(buf));

	    int N=9;
	    for(int i=0;i<N;i++)
	    {
		if(i==0)
			buf[i]=0;
		else if(i==(N-1)) 
			buf[i]=0;
		else buf[i]=i;
   	    }

	    rez = write(fd,buf,N);
	    if(rez==-1) {printf("Error write %s\n",filename); bOut=true; break;}
	    printf("\nwrited %d bytes",rez);

	    if(lseek(fd,0,SEEK_SET)<0) 
	    {perror("lseek"); break;}

	    while(1)
	    {
		char tmp[100];
		rez = read(fd,tmp,1000);

	    	if(rez==-1) {printf("Error read %s\n",filename); bOut=true; break;}

	    	if(rez==0) {printf("\nEnd of file\n"); break;}

		printf("\nread %d bytes: \n",rez);
		for(int i = 0;i<rez;i++)
			printf("%x",tmp[i]);
		 printf("\n");
	    }
	}

	getStatictic(fd);
	getPid(fd,getpid());

	close(fd);

	return 0;
}

void getStatictic(int fd)
{
	if(fd == 0) return;

	if(ioctl(fd,KBUF_IOCG_STATISTIC,(int)&statistic)<0) 
	{perror("ioctl: KBUF_IOCG_STATISTIC");}

	printf("\nstatictic: read ' %d ', write ' %d '\n", statistic.cr,statistic.cw);
}

void getPid(int fd ,int _pid)
{
	if(fd == 0) return;

	pidInfo.pid = _pid;
	if(ioctl(fd,KBUF_IOCX_IO_PID,(int)&pidInfo)<0) 
	{perror("ioctl: KBUF_IOCX_IO_PID");}

	printf("\npid information: ' %d ' ' %s '\n", pidInfo.pid,pidInfo.buf);
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye chat(signal %d)\n",sig);
	bOut=true;
    }
}