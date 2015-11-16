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

using namespace std;

int main(int argc,char* argv[], char** env)
{
	int pid = 0;
        pid = fork();

        if(!pid)// это дочерний процесс
        {
		printf("\nI was a child process!");
		
		if(unshare(CLONE_NEWUTS)==-1)
		{perror("unshare"); return 0;}
		
		char buf[] = "new_name";
		if(sethostname(buf,strlen(buf))==-1) 
		{perror("sethostname"); return 0;}
		struct utsname syspar;
		uname(&syspar);  //uname возвращает информацию о системе в структуру 
		printf("\nsysname = %s, machine = %s, nodename = %s\n",syspar.sysname, syspar.machine, syspar.nodename);
		return 0;
	}

	if(pid==-1)
	{
		perror("fork");
		return -1;
	}

	if(pid>0)
        {
		printf("\nI'm the parent process, I started the process with the PID is ' %d' and name is ", pid);
		struct utsname syspar;
		int rez = uname(&syspar);  //uname возвращает информацию о системе в структуру 
		if(rez == -1)
		{perror("parent: uname:");return 0;}

		printf("\nsysname = %s, machine = %s, nodename = %s\n",syspar.sysname, syspar.machine,syspar.nodename);
		
		return pid;
	}
}
