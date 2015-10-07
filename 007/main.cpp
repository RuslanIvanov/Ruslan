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
		int n = atoi(argv[2]);

		Param *pparam = new Param[n];
		pthread_t *pthId = new pthread_t[n];

		for(int i=0;i<n;i++)
		{
			pparam[i].num = i;
			pparam[i].left = ((N/n) - 1)*i;
			pparam[i].right = ((N/n) + 1)*i;

			pthread_create (pthId+i, NULL, funcThread, pparam+i);
		}

		void* vs=0;int s=0;
		printf("\nWait...");
		for(int i=0;i<n;i++)
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

	s=p->left + p->right;

	return (void*)s;

}
