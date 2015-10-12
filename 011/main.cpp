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

#define FIFO_IN 0
#define FIFO_OUT 1

#define BUFFER_SIZE PIPE_BUF

int pipe_fd[2];
char filename[2][BUFSIZ];
void out(int sig=0);
void * funcThread(void*);
bool bOut = false;

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

	pthread_t thId = 0;
	pthread_create(&thId, NULL, funcThread, NULL);
	printf("\nWait start task...");
	sleep(1);

	while(bOut==false)
	{
	    char buf[BUFFER_SIZE];
	    pipe_fd[FIFO_OUT] = open(&filename[FIFO_OUT][0],O_WRONLY | O_NONBLOCK);
	    if(pipe_fd[FIFO_OUT]==-1) {printf("\nError open1\n"); perror(&filename[FIFO_OUT][0]); return 0;}
		
	    printf("\nwrite: ");
	    gets(&buf[0]);
	    //scanf("%s",&buf[0]);
	    int rez = write(pipe_fd[FIFO_OUT],buf,BUFFER_SIZE);
	    if(rez==-1) {printf("Write error on pipeout");}
	
	    close(pipe_fd[FIFO_OUT]);

	}

	pthread_join(thId,NULL);
        
    }else {printf("\nError command string! Enter: 'pipein' 'pipeout'.\n");}
    printf("\n\nExit...\n");

    unlink(&filename[FIFO_OUT][0]);
    unlink(&filename[FIFO_IN][0]);
    return 0;
}

void * funcThread(void* )
{
    printf("\nRUN TASK READ");

    while(bOut==false)
    {
	char buf[BUFFER_SIZE];
	pipe_fd[FIFO_IN] = open(&filename[FIFO_IN][0],O_RDONLY);
	if(pipe_fd[FIFO_IN]==-1) { printf("\nError open2\n"); perror(&filename[FIFO_IN][0]); return 0;}
    
	int rez=read(pipe_fd[FIFO_IN],buf,BUFFER_SIZE);
	if(rez==-1) {printf("Read error on pipein");}
	printf("\n\tread: %s",buf);
	
	close(pipe_fd[FIFO_IN]);
    }

    printf("\nEXIT TASK");
    return 0;
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
