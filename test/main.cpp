#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
#include "bb"
#include "aa.cpp"
#include "tt.txt"
*/

/*
struct X
{
        int  n;
        char c;
        char ar[10];
};*/

/*
//const char ar1[3] = "abc"; //1 -
char ar2[][5] = {"abc"};   //2
char ar3[][5] = {{'a'},{'b'},{'c'}}; //3
extern const char ar4[];   //4
//*/

int main(int argc,char* argv[], char** env)
{

{	
	int x = 1;
	double y = 2, z;
	z = 2.2 + y/2;
	printf(" z %f",z);

	z = 2.2 + x/2;
	printf(" z %f",z);
	z = x + x/y;
	printf(" z %f",z);

}

{
	int n = 10;
	int* p = new int[n]; //1
	int ar[20];
	p = ar;     //2
	//delete[] p; //3 --
}
/*
{

struct A{
int* p;
};

void f()
{
A a1 = { new int };
A a2 = a1; //1
printf("a2 %x a1 %x",a2.p,a1.p);
}
}*/
/*
{
wchar_t* p = L"abcd";

printf("p %x",p);
p++;
printf("p %x",p);

}*/
/*
{
//Отметьте корректные выражения:

int* x, y;
//сформировали значения x и y
int a = *x;  //1
//int b = *y;  //2--
//int n = x-y; //3--
}*/	

/*	int x=-1;
	int y = !x;
	int z = -x;

	printf("\nx %i y %i z %i %s = %i\n",x,y,z, !x?"T":"F",!x);
*/

/*	int x = -1;
	int y = 1;
	int z = (x < y) ? x++ : y++;

	printf("%d",z);
*/

/*	int x = 5, y = 3;
	int z = x++ + y--;
	printf(" x %d y %d  z %d",x,y,z);
*/

/*
	X x = {1, 'A'};

	printf("ar: "); 
	for(int i = 0; i<10;i++)
	printf(" %d", x.ar[i]); //00..
//*/

/*
	int x=0;

	++(++x);
//	(x++)++; //-
	++(++(++x));
	(++x)++;
//*/

/*
	wchar_t d[] = L"A\nB"; //? Linux
//	char d[] = "A\nB";

	printf("s: %d",sizeof(d)/sizeof(wchar_t));
	printf("n: %d",sizeof('\n'));
//*/
/*
	int x = 0;
	if(++x)
	{
		printf("true");
	}
	else printf("false");//*/

/*
	char ar[3][5][7];
	//char* p1 = ar[1][1][1]; //1 -
	char* p2 = ar[1][1];    //2 +
	//char* p3 = ar[1];       //3 -
	//char* p4 = ar;          //4 -
*/

/*
	char *p =new char[3];
	char p2[3];
	printf("%d %d %d",p[0],p[1],p[2]); //000
	printf("\n%d %d %d",p2[0],p2[1],p2[2]);// random
	delete [] p;
*/
/*
	enum N{ ONE, TWO };
	int a = 0;
	switch(a)
	{
		case 0: // не корректно
		break;

		case ONE:
		break;

		case TWO:
		break;
	}//*/

/*
//Что можно использовать в качестве поля структуры:
//a	
	{ struct A{ int a; }; }
//b.
	{ struct A{ int a[10]; };}
//c.
	{ struct A{ A* pA; };}
//d.
	{ struct A{ int a:5; };}
//e.

	{ struct A{ int* a; };}
//f.

	{ struct B{ int b;}; //{...}
	 struct A{ B b[10]; };}

//g.
	{ struct B{int b;}; //{...}
	  struct A{ B* b;};}

//h.

	{ struct B{int b;}; //{...}
	 struct A{ B b; }; }
//i.

	{ struct A{ A a; }; }// err
	
*/

//	char *x = "ABC" + 2;

/*

struct A{ int a; };
struct B
{
	A* pA;
};


//Отметьте корректные варианты:
//Выберите один или несколько ответов:
//a.
{
A a1 = {1};//+
B b;
b.pA = &a1;
}

//b.
{
A a1 = {1};
B b = { new A }; //+
*b.pA = a1;
}

//c.
{
A a1 = {1};
A a2 = {2};
B b = {&a1};
*b.pA = a2; //++
}

//d.
{
B b = { new A };
b.pA->a = 1;//++
}

//e.
{
A a1 = {1};
B b;
*b.pA = a1;//-
}*/
/*
	double  x = 1.1;
	double *y = &x;
	double *&z =  y; //Ссылка на указатель

	printf("x %f y %p z %p",x,y,z);

	z++;

	printf("\nx %f y %p z %p",x,y,z);*/

/*
//Задано объединение:
	union A
	{	
	char c[10];
	double* d;
	};

//Отметьте корректные варианты инициализации:
	A a1 = {"abc"};      //1+
	A a2 = {new double}; //2--
	A a3 = {1.1}; //3 --
	printf("\na1 %s a3  %f %f",a1.c,a3.c[0],a3.c[1]);//
//*/

/*

	int n = 3, m = 4, k = 5;
	//char* x = new char[n][m][k];   //1-
	//char*** y = new char[3][4][5]; //2-

	char (*z)[4][5] = new char[n][4][5]; //3
	//char (*v)[4][5] = new char[n][m][k];//4-
*/

/* 	int x; int y =0;
	!x = 5;
	x+y=0;
	x=0;
*/



	getchar();
	return 0;
}


