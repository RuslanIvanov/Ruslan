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

#define FIFO_IN 0
#define FIFO_OUT 1

#define BUFFER_SIZE PIPE_BUF

int pipe_fd[2];
char filename[2][BUFSIZ];
void out(int sig=0);
bool bOut = false;

fd_set rfds;
fd_set wfds;
struct timeval tv;

void * funcThread(void*);
char buf[BUFFER_SIZE]={'\0'};
pthread_t thId;

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
	printf("\nwait open...");printf("\nwait open...");
	pipe_fd[FIFO_OUT] = open(&filename[FIFO_OUT][0], /*O_WRONLY*/ O_RDWR);//blocking
	if(pipe_fd[FIFO_OUT]==-1) {printf("\nError open1\n"); perror(&filename[FIFO_OUT][0]); return 0;}

	pipe_fd[FIFO_IN] = open(&filename[FIFO_IN][0], /*O_RDONLY*/ O_RDWR);
	if(pipe_fd[FIFO_IN]==-1) { printf("\nError open2\n"); perror(&filename[FIFO_IN][0]); return 0;}

	pthread_create(&thId, NULL, funcThread, NULL);
	sleep(1);
	    FD_SET(pipe_fd[FIFO_OUT],&wfds);
//	    FD_SET(0,&rfds);
	    FD_SET(pipe_fd[FIFO_IN],&rfds);
	int i=0;
	while(bOut==false)
	{
	   // char buf[BUFFER_SIZE]={'\0'};
//	    printf("\nwrite: ");
//            gets(&buf[0]);

	    FD_ZERO(&rfds);
	    FD_ZERO(&wfds);//obnuljat pered kagdim select

//	    FD_SET(pipe_fd[FIFO_OUT],&wfds);
//	    FD_SET(0,&rfds);
//	    FD_SET(pipe_fd[FIFO_IN],&rfds);

	    tv.tv_sec = 15;
	    tv.tv_usec = 0;

	    //printf("\nwait select...");
	    int ready = select(10/*pipe_fd[FIFO_IN]+1*/,&rfds,&wfds,NULL,&tv);
	    if(!ready){perror("select"); continue; }

	    int rez=0;

	    /*if(FD_ISSET(0,&rfds))
	    {
		printf("\nuser input: ");
		char bufr[BUFFER_SIZE]={'\0'};// вычитал в свой массив ввод пользователя
		rez=read(0,bufr,BUFFER_SIZE);
                if(rez==-1) {printf("Read error on pipein");}
                printf("\n\tread[%d]: ",rez);
                for(int i=0;i<rez;i++)
                {printf("%c",bufr[i]);}
	    }*/

	    if(FD_ISSET(pipe_fd[FIFO_OUT],&wfds))
	    {
		rez = write(pipe_fd[FIFO_OUT],buf,strlen(buf));
		if(rez==-1) {printf("Write error on pipeout");}

		//printf("\nwrited %d bytes",rez);
	    }

	    if(FD_ISSET(pipe_fd[FIFO_IN],&rfds))
	    {
		char bufr[BUFFER_SIZE]={'\0'};
		rez=read(pipe_fd[FIFO_IN],bufr,BUFFER_SIZE);
		if(rez==-1) {printf("Read error on pipein");}
		printf("\n\tread pipe[%d]: ",rez);
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

void * funcThread(void* pVoid)
{
        while(bOut==false)
	{
	      printf("\nenter msg: ");
              gets(&buf[0]);
	}
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye chat(signal %d)\n",sig);

	bOut=true;

	//unlink(&filename[FIFO_OUT][0]);
        //unlink(&filename[FIFO_IN][0]);
	exit(0);
    }
}
