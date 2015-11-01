#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/poll.h>

#define FIFO_IN 0
#define FIFO_OUT 1

#define BUFFER_SIZE PIPE_BUF

int pipe_fd[2];
char filename[2][BUFSIZ];
void out(int sig=0);
bool bOut = false;
struct pollfd fds[3];

char buf[BUFFER_SIZE]={'\0'};

int main(int argc,char* argv[], char** env)
{
    signal (SIGTERM, out);
    signal (SIGINT, out);

    if(argc==3)
    {
	char buf[BUFFER_SIZE];
	strcpy(&filename[FIFO_IN][0],"/tmp/");
	strcpy(&filename[FIFO_OUT][0],"/tmp/");
	strcat(&filename[FIFO_IN][0],argv[1]);
	strcat(&filename[FIFO_OUT][0],argv[2]);

	printf("\nfifo in ' %s ', fifo out ' %s ', exit ' Ctrl+C '\n",&filename[FIFO_IN][0],&filename[FIFO_OUT][0]);

	for(int i=0;i<2;i++)
	if(access(&filename[i][0],F_OK)==-1)
	{
	    int rez =  mkfifo(&filename[i][0],0777);
	    if(rez!=0) 
	    {
		char tmp[BUFSIZ];
		printf("\nError mkfifo %s",&filename[i][0]); 
		sprintf(tmp,"%s: ",&filename[i][0]);
		perror(tmp); 

		return 0;
	    }
	    printf("\nmkfifo %s",&filename[i][0]);
	}
	sleep(1);

	//printf("\nwait open...");
	fds[2].fd = open(&filename[FIFO_OUT][0], /*O_WRONLY*/ O_RDWR);//blocking with O_WRONLY
	fds[2].events = POLLOUT;
	if(fds[2].fd==-1) {printf("\nError open1\n"); perror(&filename[FIFO_OUT][0]); return 0;}

	fds[1].fd = open(&filename[FIFO_IN][0], /*O_RDONLY*/ O_RDWR);
	fds[1].events = POLLIN;
	if(fds[1].fd==-1) { printf("\nError open2\n"); perror(&filename[FIFO_IN][0]); return 0;}

	fds[0].fd = 0; //stdin
	fds[0].events = POLLIN;

	sleep(1);

	int i=0;
	while(bOut==false)
	{
	   int ready = poll(&fds[0], 3, 5000);
	   if(ready==0){perror("poll"); continue; }
	   if(ready<0){perror("poll"); break; }

	    int rez=0;
	    if( fds[0].revents & POLLIN )
	    {
		char bufr[BUFFER_SIZE]={'\0'};// вычитал в свой массив ввод пользователя
		rez=read(fds[0].fd,bufr,BUFFER_SIZE);
                if(rez==-1) {printf("Read error on pipein");}
            
		int i=0;
                for(;i<rez;i++)
                {
			buf[i] = bufr[i];
		}
		buf[i]= '\0';

		rez = write(fds[2].fd,buf,strlen(buf));
		if(rez==-1) {printf("Write error on pipeout");}
	    }

	    if( fds[1].revents & POLLIN )
	    {
		char bufr[BUFFER_SIZE]={'\0'};
		rez=read(fds[1].fd,bufr,BUFFER_SIZE);
		if(rez==-1) {printf("Read error on pipein"); break;}
		printf("\n\t<-[%d]: ",rez);
		for(int i=0;i<rez;i++)
		{printf("%c",bufr[i]);}
	    }
	}

	close(pipe_fd[FIFO_OUT]);
	close(pipe_fd[FIFO_IN]);

    }else {printf("\nError command string! Enter: 'pipein' 'pipeout'.\n");}
    printf("\n\nExit...\n");

    unlink(&filename[FIFO_OUT][0]);
    unlink(&filename[FIFO_IN][0]);

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
