#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int x;

int main()
{
	int y = x;
	int x = 5;
	y += ::x;
	{
		int x = 10;
		y += ::x;
	}

	y += ::x;

	printf(" y %d",y);

//зависание
int n = 0;
do
{
	n++;
	printf(" y %d",y);
}while(n = 5);

	return 0;
}
