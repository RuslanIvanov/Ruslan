#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

unsigned int size=256;
char filename[BUFSIZ];
void out(int sig=0);
bool bOut = false;
//char buf[BUFSIZ]={'\0'};
struct stat statbuf; 

int comp(const void *i, const void *j)
{
 	return *(int *)i - *(int *)j;
}

int main(int argc,char* argv[], char** env)
{
    signal (SIGTERM, out);
    signal (SIGINT, out);

    int opt;
    while((opt= getopt(argc, argv, "s:f:h")) != -1)
    switch(opt)
    {
        case 'f': sscanf(optarg,"%s",&filename[0]); break;
        case 's': sscanf(optarg,"%d",&size); break;
        case 'h':default:
		printf("\nargv:\n");
		printf("\t-f\t file name\n");
		printf("\t-s\t size block\n");
        	return 0;
	}

	printf("\nfile in ' %s ', size ' %d ', exit ' Ctrl+C '\n",filename,size);	

	int fds = open(filename, O_RDWR);
	if(fds==-1) {perror("open"); return 0;}

	if (fstat(fds, &statbuf)<0) /* определить размер входного файла */
	    perror("fstat");

	void *pmmap = mmap(0,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fds, 0);
	if(pmmap == MAP_FAILED)
	{
		perror("mmap"); return 0;
	}

	/*int i=0;
	while(i<=statbuf.st_size)
	{	   
		i=i+size;
	}*/

	qsort(pmmap, statbuf.st_size, size, comp);

	munmap(0,statbuf.st_size);
	close(fds);

    printf("\n\nExit...\n");

    return 0;
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye mmap(signal %d)\n",sig);
	bOut=true;
   }
}
