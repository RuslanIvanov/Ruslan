#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <list>
#include <iostream>
#include <sys/utsname.h>
#include <unistd.h>
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#include <sched.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>

using namespace std;
int N=3;
int i=0;

int main(int argc,char* argv[], char** env)
{
	char bufCatName[BUFSIZ];

	for(int i = 0;i<N;)
	{
	    sprintf(bufCatName,"./catMountProc_%d",i);
	    mkdir(bufCatName,555);

	    int pid = 0;
	    pid = fork();

    	    if(!pid)// это дочерний процесс
    	    {
		printf("\nI was a child process:\n");
		
		if(unshare(CLONE_NEWPID)==-1)
		{perror("unshare"); return 0;}

		mount("proc",bufCatName,"proc",0,NULL);
		printf("\npid child %d\n", getpid());
		return 0;
	    }

		if(pid==-1)
		{
		    perror("fork");
		    return -1;
		}

		if(pid>0)
    		{
		    printf("\nI'm the parent process ' %d '", pid);
		    i++;
		    //return pid;
		}
	}
}
