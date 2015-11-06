#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>

#include "lib/libsem.h"

void *shared_memory = (void *)0;
struct memFormat
{
	pid_t pidMaster;
	unsigned int sizeItem;
	unsigned int count;
	int max; 
};
memFormat memf;
void out(int sig);
bool bOut;
int *pitem;

void delay(int mc)
{
    usleep(mc*1000);// suspend execution for microsecond intervals
}

int main()
{
    signal (SIGTERM, out);
    signal (SIGINT, out);

    int shmid;
    //Какой размер выделять как узнать?
    shmid = shmget((key_t)1234, sizeof(struct memFormat), 0666 /*| IPC_CREAT*/);

    if (shmid == -1)  {perror("shmget");return 0;}

    shared_memory = shmat(shmid, (void *)0, SHM_RDONLY);
    if (shared_memory == (void *)-1) {perror("shmat");return 0;}

    pitem = (int*)shared_memory+sizeof(struct memFormat);
    printf("Monitor memory attached at %p,data attached at %p\n", shared_memory,pitem);

    memcpy(&memf,shared_memory, sizeof(struct memFormat));
	
    int sem_id = createSem(1,12345);
    if(sem_id==-1) {perror("createSem"); return 0;}
	
    if(set_semvalue(sem_id,0)==-1) {perror("set_semvalue"); return 0;}

    printf("memory pid %d:\n sizeItem %d, count %d, max %d",memf.pidMaster,memf.sizeItem,memf.count,memf.max);
    while(bOut==false) 
    {        
        //sleep(1);  
	delay(100);
	int count1=0;
	if(!semaphore_p(sem_id)) {perror("semaphore_p");break;}
	memcpy(&memf,shared_memory, sizeof(struct memFormat));	

	for(int vi=0;vi<memf.count;vi++)
	{
		if(*pitem==1) count1++;
		//printf("%d.",*pitem);
		pitem++;
	}

	if(!semaphore_v(sem_id)) {perror("semaphore_v");break;}
	printf("\nnumber '1': %d.",count1);          
        
    }

    if (shmdt(shared_memory) == -1) {perror("shmat");}
    del_semvalue(sem_id,0);
    printf("\nExit...\n");
}

void out(int sig)
{
    if(sig==SIGTERM || sig == SIGINT)
    {
	printf("\nGoodbye (signal %d)\n",sig);
	bOut=true;
    }
}
