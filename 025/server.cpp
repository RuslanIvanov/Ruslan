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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <vector>
#include <string>
#include <map>

using namespace std;

#define MAX_CONNECT 100

void out(int sig=0);
bool bOut = false;

int main(int argc, char* argv[])
{

    signal (SIGTERM, out);
    signal (SIGINT, out);

    struct sockaddr_in addr;
  
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port); 

    int sock;
   
    char buf[BUFSIZ];
    int bytes_read;

    int listener = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //int optval = 1;
    //if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)==-1){perror("SO_REUSEADDR:"); }
    
    iConnect=0;
    while(bOut==false)
    {


	iConnect++;
      
    }
    
    return 0;
}

void out(int sig)
{
    if(sig==SIGTERM||sig == SIGINT)
    {
	printf("\nGoodbye server'\n");
	bOut=true;
    }
}
