#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void * funcThread(void*);
struct Param
{
    int num;
    double left;
    double right;
};
int S;

int main(int argc,char* argv[], char** env)
{
	if(argc>1)
	{
		int N = atoi(argv[1]);

		Param *pparam = new Param[N];
		pthread_t *pthId = new pthread_t[N];

		for(int i=0;i<N;i++)
		{
			pparam[i].num = i;
			pthread_create (pthId+i, NULL, funcThread, pparam+i);
		}

		void* vs=0;int s=0;
		printf("\nWait...");
		for(int i=0;i<N;i++)
		{
			pthread_join(pthId[i],&vs);
			s=(int)vs;
			S+=s;
		}

		delete [] pthId; 
		delete [] pparam;
	}

	printf("\nS = %d, Exit...\n",S);
	return 0;
}

void * funcThread(void* inputPar)
{
	int s=0;
	Param *p = (Param*)inputPar;
	printf("\n\trun thread %d\n",p->num);

	s=1;

	return (void*)s;

}
