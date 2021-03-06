#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include "driver2/ioctl_netkbuf.h"

int  fd;
char filename[BUFSIZ];

void getStatictic(int);

STATISTIC_RW statistic;

int main(int argc,char* argv[], char** env)
{

	strcpy(&filename[0],"/dev/chnet");

	printf("\nfile ' %s ', exit ' Ctrl+C '\n",&filename[0]);

	fd = open(&filename[0],O_RDWR);

	if(fd==-1) {printf("\nError open %s\n",filename); return 0;}

	for(int t=0;t<10;t++)
	{
		sleep(1);
		unsigned char tmp[600];
		int rez = read(fd,tmp,100);
    		if(rez==-1) {printf("\nError read %s\n",filename);}

		printf("\nread %d bytes: \n",rez);
		for(int i = 0;i<rez;i++)
			printf("  %02x",tmp[i]);
		printf("\n");

		getStatictic(fd);
	}

        close(fd);
	printf("\nClose %s.\n",&filename[0]);

	return 0;
}


void getStatictic(int fd)
{
	if(fd == 0) return;

	if(ioctl(fd,KBUF_IOCG_STATISTIC,&statistic)<0) 
	{perror("ioctl: KBUF_IOCG_STATISTIC");}

	printf("\nstatictic: read ' %d ', write ' %d '\n", statistic.cr,statistic.cw);
}


