#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void * funcThread(void*)
struct Peram
{
    int num;
    double left;
    double right;
};

int main(int argc,char* argv[], char** env)
{
	pthred_t thId = 0;
	pthread_create (&thId, NULL, funcThread, p);
	return 0;
}
