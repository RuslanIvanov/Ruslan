#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int pid;
int sig;
int rez;
extern const char * const sys_siglist[];
int main(int argc,char* argv[], char** env)
{
       if(argc==3)
       {
	    pid = atoi(argv[1]);
	    sig = atoi(argv[2]);
	    if(pid>0)
	    	{rez = kill(pid, sig);}
	    else
		{printf("\nInvalid pid, plesae enter pid >0! Exit...\n"); return 0;}
		
		if(rez == -1)
		{
			perror("kill");
			return 0;
		}

		printf("\nSend signal ' %s ' to pid %d. Exit...\n",strsignal(sig),pid);	
       		return 0;
	}

	printf("\nIncorrect input parameters! Exit...\n");

        return 0;
}

