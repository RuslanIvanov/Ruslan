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

	label1:

	if(pid==0 && i<N)// это дочерний процесс
	{
		sprintf(bufCatName,"./catMountProc_%d",i);
                mkdir(bufCatName,555);
		//i++;
		pid = fork();// exception, sig = SIGCHLD ???

		if(pid==0)
		{///рекурсия должна быть
			printf("\nI was a child process:\n");

			if(unshare(CLONE_NEWPID)==-1)
			{ perror("unshare"); return 0; }

			sleep(1);
			if(mount("proc",bufCatName,"proc",0,NULL)==-1)
			{ perror("mount"); return 0; }

			printf("Mounting procfs at %s\n",bufCatName);
			sleep(1);
			printf("\npid child %d\n", getpid());
			i++;

			goto label1;

		}

		if(pid==-1)
		{perror("fork"); return -1;}

		if(pid>0)
		{
			printf("\nI'm the parent process ' %d ',\nwait...", pid);
			if (waitpid(pid, NULL, 0) == -1) 
			perror("waitpid");
			i=N;
		}
	}else {printf("Final child sleeping\n"); execlp("sleep", "sleep", "1000", (char *) NULL);}
}
