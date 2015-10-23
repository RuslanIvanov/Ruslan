#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>

int funcThread(void*);
struct Param
{
    double a;
    double h;
    int i;
};

double S;
double *pS;

double f(double x)
{
    static int r;
      return sin(x);
}

int main(int argc,char* argv[], char** env)
{
	if(argc==3)
	{
		double a = 0; //Левая граница
		double b = atoi(argv[2]);// правая граница
		int n = atoi(argv[1]);// кол_во разбиений

		if(n<=0){printf("\nError: number of partitions\n\n"); return 0;} 
		if(n>300){printf("\nError: max number of partitions 300 \n\n"); return 0;} 

		double h=0.0;
		Param *pparam = NULL;
		pthread_t *pthId = NULL;

    		h=(b-a)/(double)n; // шаг интегрирования

		pparam = new Param[n];
		pthId = new pthread_t[n];
		pS = new double [n];
		void* child_stack=new Param[100];
		int flags=CLONE_VM;
		for(int ii=0;ii<n-1;ii++)
    		{
			pparam[ii].a = a;
			pparam[ii].h = h;
			pparam[ii].i = ii;
			pthId[ii] = clone(funcThread, child_stack, flags,pparam+ii); 
			if(pthId[ii]==-1){ perror(""); continue;}
			//pthread_create(pthId+ii, NULL, funcThread, pparam+ii);
    		}

		printf("\nstep %6.3f",h);
		printf("\nWait...");

		/*for(int i=0;i<n-1;i++)
		{pthread_join(pthId[i],NULL);}*/
		int stat;
		for(int i=0;i<n-1;i++)
		{waitpid(pthId[i],&stat,2);}

		printf("\ncalculated!\n");
		for(int i=0;i<n-1;i++)
                {
                	S+=pS[i];
                }

		S=S*h;
		delete [] pthId; 
		delete [] pparam;
		delete [] pS;
		delete [] child_stack;

	}else {printf("\nError: set command string: [number of partitions] [right limit]\n\n"); return 0;} 

	printf("\nS = %6.3f\nExit!\n\n",S);
	return 0;
}

int funcThread(void* inputPar)
{
	double x=0.;
	Param *p = (Param*)inputPar;

	x=(p->a)+(p->i)*(p->h);
        pS[p->i]=f(x);
	//printf("\nS[%d] = %6.3f",p->i,pS[p->i]);
	return 0;
}

