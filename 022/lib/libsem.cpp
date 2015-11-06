
#include "libsem.h"

int createSem(int num_sem,key_t key)
{
	return semget(key,num_sem, 0666| IPC_CREAT);
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
{//wait
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
{//run
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
