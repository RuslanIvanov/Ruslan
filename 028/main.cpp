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
#include <sys/wait.h>

using namespace std;
int N=3;
int i=0;
char bufCatName[BUFSIZ];
int pid = 0;
int main(int argc,char* argv[], char** env)
{
	    if(pid==0 && i<N)// это дочерний процесс
	    {
		pid = fork();
		i++;
		if(pid==0) 
		{
			printf("\nI was a child process:\n");

			if(unshare(CLONE_NEWPID)==-1)
			{perror("unshare"); return 0;}

			mount("proc",bufCatName,"proc",0,NULL);
			printf("\npid child %d\n", getpid());

			sprintf(bufCatName,"./catMountProc_%d",i);
	   	     	mkdir(bufCatName,555);
		}
	    }else{ sleep(1000); }

	    if(pid==-1)
	    {perror("fork"); return -1;}

	    if(pid>0)
    		{
			printf("\nI'm the parent process ' %d '", pid);
		    
			if (waitpid(pid, NULL, 0) == -1) 
			perror("waitpid");
		}
}
