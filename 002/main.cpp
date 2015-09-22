#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char* argv[], char** env)
{
	char buf[100];

	while(*env++) //*env != NULL
	{
		//printf("\n%s",*env);
		char * p = strstr(*env,"PWD=");
		if(*env==p) 
		{
			printf("\ncurrent path: %s\n",p+4);
			break;
		}
	}

    	return 0;
}
