#ifndef _LIBSEM_H_
#define _LIBSEM_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
	// union semun is defined by including <sys/sem.h> 
#else
	// according to X/OPEN we have to define it ourselves
union semun 
{
	int val;                  // value for SETVAL 
	struct semid_ds *buf;     // buffer for IPC_STAT, IPC_SET 
	unsigned short *array;    // array for GETALL, SETALL 
                           // Linux specific part: 
	struct seminfo *__buf;    // buffer for IPC_INFO 
};
#endif//*/

int  set_semvalue(int sem_id,int num_sem);
void del_semvalue (int sem_id,int num_sem);
int  semaphore_v(int sem_id);
int  semaphore_p(int sem_id);
int  createSem(int num_sem,key_t key);

struct memFormat
{
	pid_t pidMaster;
	unsigned int sizeItem;
	unsigned int count;
	int max; 
};

#endif
