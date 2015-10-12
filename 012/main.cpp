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
#include <vector>

//th1 - write 1; th2 - write 0;  for -read 
using namespace std;

vector <int> v;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void out(int sig);
bool bOut;

void * funcThreadW(void* pi);
void * funcThreadR(void* pi);
int main(int argc,char* argv[], char** env)
{
	signal (SIGTERM, out);
	if(argc>1 && argc<=4)
	{
	    int nr =  atoi(argv[1]);
	    int nw =  atoi(argv[2]);
	    int max = atoi(argv[3]);

	   
	    pthread_mutex_init(&mutex, NULL);

	    v.reserve(max);
	    pthread_t* thId= new pthread_t[nr+nw];

	    for(int i=0;i<nw;i++)
	    pthread_create(thId+i, NULL, funcThreadW, &i);
	
	    for(int i=nw;i<nr+nw;i++)
	    pthread_create(thId+i, NULL, funcThreadR, &i);

	    while(bOut==false)
	    {
	   
	    	printf("\nvector has 1: \n");
		int count1=0;
		pthread_mutex_lock(&mutex);
	    	for(int vi=0;vi<v.size();vi++)
	    	{	
			if(v[vi]==1) count1++;
			//printf("%d.",v[vi]);

	    	}
		pthread_mutex_unlock(&mutex);
		printf("%d.",count1);
	    }

	    for(int i=0;i<(nr+nw);i++)
		pthread_join(thId[i],NULL);

	    pthread_mutex_destroy(&mutex);
	
		delete [] thId;
	}
	printf("\n\nExit...\n");

    return 0;
}

void * funcThreadW(void* pi)
{
	int* pipi = (int*)pi;
	printf("\nRUN TASK READ %d",*pipi);
	while(bOut==false)
	{
	    pthread_mutex_lock(&mutex);
	//    for(int vi=0;vi<v.size();vi++)
	    v.push_back(1);
	    pthread_mutex_unlock(&mutex);
	}
	printf("\nEXIT TASK %d",*pipi);
	return 0;
}

void * funcThreadR(void* pi)
{
    int* pipi = (int*)pi;
    printf("\nRUN TASK READ %d",*pipi);
    while(bOut==false)
    {
	pthread_mutex_lock(&mutex);
	//for(int vi=0;vi<v.size();vi++)
	//{
		//if(v[vi]==1)
	    	//	v[vi]=0;
		v.pop_back();
	//}
	pthread_mutex_unlock(&mutex);
    }
    printf("\nEXIT TASK %d",*pipi);
    return 0;
}

void out(int sig)
{
    if(sig==SIGTERM)
    {
	printf("\nGoodbye chat(signal %d)\n",sig);
	bOut=true;
    }
}
