
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct A
{
	char ar[10];
};

//Отметьте корректные варианты обращения к полям структуры:
//Выберите один или несколько ответов:
//a.
void fa(A* pA)
{
//	pA->ar = "ABC";
}

//b.
void fb(A* pA)
{
	pA->ar[2] = 'A';
	printf("fb: '%c' ",pA->ar[2]);
}

//c.
void fc(A* pA)
{
	*pA->ar = 'A';
	printf("fc: '%c' ",*pA->ar);
}


int main()
{
	A a;
	fa(&a);
	fb(&a);
	fc(&a);

	return 0;
}


