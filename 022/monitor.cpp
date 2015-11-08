#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>

#include "lib/libsem.h"

void *shared_memory = (void *)0;

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
    /* искать сегмент, соответствующий ключу key*/
    shmid = shmget((key_t)1234, sizeof(struct memFormat), 0666);

    if (shmid == -1)  {perror("shmget");return 0;}

    shared_memory = shmat(shmid, (void *)0, SHM_RDONLY);
    if (shared_memory == (void *)-1) {perror("shmat");return 0;}

    int sem_id = createSem(1,12345);
    if(sem_id==-1) {perror("createSem"); return 0;}
	
    if(set_semvalue(sem_id,0)==-1) {perror("set_semvalue"); return 0;}

    semaphore_p(sem_id);
    memcpy(&memf,shared_memory, sizeof(struct memFormat)); 
    semaphore_v(sem_id);

    if (shmdt(shared_memory) == -1) {perror("shmat"); return 0;}

    int size = sizeof(struct memFormat)+(memf.max*memf.sizeItem);
    printf("Moniotr attache mem size: %d + %d=%d",sizeof(struct memFormat),(memf.max*memf.sizeItem),size);

    shmid = shmget((key_t)1234, size, 0666);

    if (shmid == -1)  {perror("shmget2");return 0;}

    shared_memory = shmat(shmid, (void *)0, SHM_RDONLY);
    if (shared_memory == (void *)-1) {perror("shmat2");return 0;}

    pitem = (int*)shared_memory+sizeof(struct memFormat);
    printf("Monitor memory attached at %p,data attached at %p\n", shared_memory,pitem);

    printf("memory pid %d:\n sizeItem %d, count %d, max %d",memf.pidMaster,memf.sizeItem,memf.count,memf.max);
    sleep(1);
    while(bOut==false) 
    {        
	pitem = (int*)shared_memory+sizeof(struct memFormat);

	delay(500);
	
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

    if (shmdt(shared_memory) == -1) {perror("shmat2");}
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
