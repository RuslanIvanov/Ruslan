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
#include <sys/poll.h>

//#define ALTER
#ifdef ALTER
	#include "alternativeDriver/ioctl_kbuf.h"
#else
	#include "driver/ioctl_kbuf.h"
#endif

int  fd;
char filename[BUFSIZ];
void out(int sig=0);
char buf[BUFSIZ];
bool bOut = false;
void getStatictic(int);
void getPid(int,int);
void getIrq(int);
void testPoll();

PID_INFO pidInfo;
STATISTIC_RW statistic;
int numTest=0;

int main(int argc,char* argv[], char** env)
{

	signal (SIGTERM, out);
	signal (SIGINT, out);

	strcpy(&filename[0],"/dev/chkbuf");

	printf("\nfile ' %s ', exit ' Ctrl+C '\n",&filename[0]);

	for(;numTest<2;numTest++)
	{

	if(numTest==1)
	{
		printf("\ntest read-write & O_NONBLOCK %s:",&filename[0]);
		fd = open(&filename[0],O_RDWR|O_NONBLOCK); 
	}

	if(numTest==0)
	{
		printf("\ntest read-write %s:",&filename[0]);
                fd = open(&filename[0],O_RDWR); 
	}

//	fd = open(&filename[0],O_RDONLY);
//	fd = open(&filename[0],O_WRONLY);

	if(fd==-1) {printf("\nError open %s\n",filename); return 0;}

	int countStr=0;
	//while(bOut==false)
	while(countStr<8)
	{
	    sleep(1);
	    int rez=0;

	    sprintf(buf,"It is number string %d\n", countStr++);
	    rez = write(fd,buf,strlen(buf)+1);

	    if(rez==-1) {printf("Error write %s\n",filename); bOut=true; break;}
	    printf("\nwrited %d bytes",rez);

	    printf("\nLSEEK: set pos %d",(numTest)*10);
	    if(lseek(fd,(numTest)*10,SEEK_SET)<0) 
	    {perror("lseek"); bOut=true; break;}
		char tmp[70];
		rez = read(fd,tmp,70);

	    	if(rez==-1) {printf("\nError read %s\n",filename); bOut=true; break;}

	    	if(rez==0) {printf("\nEnd of file\n"); bOut=true; break;}

		printf("\nread %d bytes: \n",rez);
		for(int i = 0;i<rez;i++)
			printf("%c",tmp[i]);
		 printf("\n");
	}

	if(numTest==1)
	{
		printf("\nTest poll, write msg: 'It is test 'poll' / 'select'\n");
		if(lseek(fd,0,SEEK_SET)<0) 
		{perror("lseek");} 
		char tmp[BUFSIZ];
		sprintf(tmp,"It is test 'poll' / 'select'\n");
         	int rez = write(fd,tmp,strlen(tmp)+1);
		if(rez==-1) {printf("Error test 'poll' / 'selsct'\n");}

	}

	getStatictic(fd);
	getPid(fd,getpid());
	getIrq(fd);

        close(fd);
	printf("\nClose %s.\n",&filename[0]);

	}

	sleep(1);

	testPoll();

	return 0;
}

void testPoll()
{
	struct pollfd fds;

	printf("\ntest read with poll %s:",&filename[0]);
	int  fd = open(&filename[0],O_RDONLY); 
	if(fd==-1) {perror("open for poll"); return ;}

	fds.fd=fd;
	fds.events = POLLIN;

	printf("\ntest poll...\n");
	for(int ipoll=0;ipoll<60;ipoll++)
	{
		int ready = poll(&fds, 1, 50);
        	if(ready==0){perror("poll"); continue; }
        	if(ready<0){perror("poll"); break; }

        	int rez=0;
        	if( fds.revents & POLLIN )
        	{
                	char bufr[BUFSIZ];
                	rez=read(fd,bufr,BUFSIZ);
			if(rez<0) {perror("readpoll"); return;}

			printf("\nreadpoll %d bytes: \n",rez);
                	for(int i = 0;i<rez;i++)
                		printf("%c",bufr[i]);
                	printf("\n");
		}
	}

	close(fd);
}

void getStatictic(int fd)
{
	if(fd == 0) return;

	if(ioctl(fd,KBUF_IOCG_STATISTIC,&statistic)<0) 
	{perror("ioctl: KBUF_IOCG_STATISTIC");}

	printf("\nstatictic: read ' %d ', write ' %d '\n", statistic.cr,statistic.cw);
}

void getPid(int fd ,int _pid)
{
	if(fd == 0) return;

	pidInfo.pid = _pid;
	if(ioctl(fd,KBUF_IOCX_IO_PID,&pidInfo)<0) 
	{perror("ioctl: KBUF_IOCX_IO_PID");}

	printf("\nprocess information: ' %d ' ' %s '\n", pidInfo.pid,pidInfo.buf);
}

void getIrq(int fd)
{
	if(fd == 0) return;

	unsigned int numIrq=0;
	if(ioctl(fd,KBUF_IOCG_GETNUMIRQ,&numIrq)<0) 
	{perror("ioctl: KBUF_IOCG_GETNUMIRQ");}

	printf("\nnum irg: ' %d '\n", numIrq);
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye chat(signal %d)\n",sig);
	bOut=true;
    }
}
