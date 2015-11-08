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
#include <sys/shm.h>
#include <sys/syscall.h> 
#include "lib/libsem.h"
//th1 - write 1; th2 - read 0;  for -read count
using namespace std;

void out(int sig);
bool bOut;
void *shared_memory = (void *)0;

memFormat memf;
int shmid;
int *pitem;
int sem_id ;

long  getTid()
{
    long tid;
    tid = syscall(SYS_gettid);
    return tid;
}


void * funcThreadW(void* pi);
void * funcThreadR(void* pi);
int main(int argc,char* argv[], char** env)
{
	signal (SIGTERM, out);
	signal (SIGINT, out);

	if(argc>1 && argc<=4)
	{
	    int nr =  atoi(argv[1]);
	    int nw =  atoi(argv[2]);
	    int max = atoi(argv[3]);

	    shmid = shmget((key_t)1234, sizeof(struct memFormat)+(max*sizeof(int)), 0666 | IPC_CREAT);

    	    if (shmid == -1) 
	    {perror("shmget");return 0;}

    	    shared_memory = shmat(shmid, (void *)0, 0);
    	    if (shared_memory == (void *)-1) 
	    {perror("shmat");return 0;}
		
	    pitem = (int*)shared_memory+sizeof(struct memFormat);
	    memf.pidMaster = getpid();
	    memf.sizeItem = sizeof(int);
	    memf.count = 0;
	    memf.max=max;	   
	    memcpy(shared_memory, &memf,sizeof(struct memFormat));

	    printf("Memory attached at %p,data attached at %p\n", shared_memory,pitem);

	    sem_id = createSem(1,12345);
	    if(sem_id==-1) {perror("createSem"); return 0;}
	
    	    if(set_semvalue(sem_id,0)==-1) {perror("set_semvalue"); return 0;}

	    pthread_t* thId= new pthread_t[nr+nw];

	    for(int i=0;i<nw;i++)
	    pthread_create(thId+i, NULL, funcThreadW, (void*)&memf);

	    for(int i=nw;i<nr+nw;i++)
	    pthread_create(thId+i, NULL, funcThreadR, (void*)&memf);

            printf("\nmem has size %d\n",sizeof(struct memFormat)+(max*sizeof(int)));

	    printf("\nwait threads...");
	    for(int i=0;i<(nr+nw);i++)
		pthread_join(thId[i],NULL);

	    del_semvalue(sem_id,0);
	
	    delete [] thId;

	    if(shmdt(shared_memory) == -1) {perror("shmdt");}
	    if(shmctl(shmid, IPC_RMID, 0) == -1) {perror("shmctl");}
    }else {printf("\n Error. Please, set command string 'nr' 'nw' 'maxth'");}

    printf("\n\nExit...\n");

    return 0;
}

void * funcThreadW(void* param)
{
        struct memFormat *p = (struct memFormat *)param;
	printf("\nRUN TASK WRITE %ld",getTid());
	while(bOut==false)
	{
	
	   if(!semaphore_p(sem_id)) {perror("semaphore_p");break;}

	    *(pitem + p->count) =  1;
	    printf("\nw:%d",*(pitem + p->count));
	    if(p->count < p->max) p->count++;
	    memcpy(shared_memory, &memf,sizeof(struct memFormat));
	   
	    if(!semaphore_v(sem_id)) {perror("semaphore_v");break;}

	    sleep(1);
	}
	printf("\nEXIT TASK %ld",getTid());
	return 0;
}

void * funcThreadR(void* param)
{
    struct memFormat *p = (struct memFormat *)param;
    printf("\nRUN TASK READ  %ld",getTid());
    while(bOut==false)
    {

	if(!semaphore_p(sem_id)) {perror("semaphore_p");break;}

	*(pitem + p->count)=0;
        printf("\nr %d",*(pitem + p->count));
	if(p->count>0) p->count--; 
	memcpy(shared_memory, &memf,sizeof(struct memFormat));

	if(!semaphore_v(sem_id)) {perror("semaphore_v");break;}

	sleep(1);
    }
    printf("\nEXIT TASK %ld",getTid());
    return 0;
}

void out(int sig)
{
    if(sig==SIGTERM || sig == SIGINT)
    {
	printf("\nGoodbye (signal %d)\n",sig);
	bOut=true;
    }
}
