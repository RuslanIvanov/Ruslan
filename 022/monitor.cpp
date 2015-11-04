#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>

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
	
    printf("memory pid %d:\n sizeItem %d, count %d, max %d",memf.pidMaster,memf.sizeItem,memf.count,memf.max);
    while(bOut==false) 
    {
        
        sleep(1);  
	int count1=0;
memcpy(&memf,shared_memory, sizeof(struct memFormat));	
	//sem и читать count постоянно
	for(int vi=0;vi<memf.count;vi++)
	{
		if(*pitem==1) count1++;
		printf("%d.",*pitem);
		pitem++;
	}

	//sem
	printf("\nnumber '1': %d.",count1);          
        
    }

    if (shmdt(shared_memory) == -1) {perror("shmat");}
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
