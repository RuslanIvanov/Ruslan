#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
int val;                  /* value for SETVAL */
struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
unsigned short *array;    /* array for GETALL, SETALL */
                           /* Linux specific part: */
struct seminfo *__buf;    /* buffer for IPC_INFO */
};
#endif

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

int  set_semvalue(int sem_id,int num_sem);
void del_semvalue (int sem_id,int num_sem);
int  semaphore_v(int sem_id);
int  semaphore_p(int sem_id);
int  createSem(int num_sem);
//ф. семафоры поместить в библиотеку
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
	
    int sem_id = createSem(1);
    if(sem_id==-1) {perror("createSem"); return 0;}
	
    if(set_semvalue(sem_id,0)==-1) {perror("set_semvalue"); return 0;}

    printf("memory pid %d:\n sizeItem %d, count %d, max %d",memf.pidMaster,memf.sizeItem,memf.count,memf.max);
    while(bOut==false) 
    {        
        sleep(1);  
	int count1=0;
	if(!semaphore_p(sem_id)) {perror("semaphore_p");continue;}
	memcpy(&memf,shared_memory, sizeof(struct memFormat));	

	for(int vi=0;vi<memf.count;vi++)
	{
		if(*pitem==1) count1++;
		printf("%d.",*pitem);
		pitem++;
	}

	if(!semaphore_v(sem_id)) {perror("semaphore_v");continue;}
	printf("\nnumber '1': %d.",count1);          
        
    }

    if (shmdt(shared_memory) == -1) {perror("shmat");}
    del_semvalue(sem_id,0);
    printf("\nExit...\n");
}

int createSem(int num_sem)
{
	return semget((key_t)12345,num_sem, 0666| IPC_CREAT);
}

int set_semvalue(int sem_id,int num_sem)
{
    union semun sem_union;
    
    sem_union.val = 1;
    return semctl(sem_id,/*0*/num_sem,SETVAL,sem_union);
}

void del_semvalue (int sem_id,int num_sem)
{
	union semun sem_union;
	if(semctl(sem_id,/*0*/num_sem,IPC_RMID,sem_union) == -1)
	   perror("semctl");
}

int semaphore_p(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0 ;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    
    if(semop(sem_id, &sem_b, 1) == -1)
    {
	fprintf(stderr, "semaphore_p failed\n");
	return 0;
    }
    
    return 1;
}

int semaphore_v(int sem_id)
{
	struct sembuf sem_b;
	
	sem_b.sem_num = 0;
	sem_b.sem_op  = 1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b,1)== -1)
	{
	    fprintf(stderr, "semaphore_v failure\n");
	    return 0;
	}
	return 1;
}

void out(int sig)
{
    if(sig==SIGTERM || sig == SIGINT)
    {
	printf("\nGoodbye (signal %d)\n",sig);
	bOut=true;
    }
}
