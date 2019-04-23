#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace X{ int x; }

int main()
{

	using namespace X;
	int y = x;
	int x = 1;
	y += x; //1 y=y+x;
	{
		int x = 10;
		y += x;
	}

	y += X::x;
	printf("\ny= %d",y);
	return 0;

}


