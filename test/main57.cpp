//Отметьте корректные варианты вызова функции f:


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void f(int&);

int main()
{
int n = 1;
int* p = &n;
f(n);  //1
f(*p); //2
//f(1);  //3--
}

void f(int&){}
