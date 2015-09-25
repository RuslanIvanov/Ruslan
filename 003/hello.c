
#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main()
{
	printf("\n\tHello world\n");
	void *library_handler;
	void (*pmyprint)();

	library_handler = dlopen("./libprint.so",RTLD_LAZY);
	if (!library_handler)
	{
		printf("dlopen() error: %s\n", dlerror());
		exit(1);
	}

	pmyprint = dlsym(library_handler,"myprint");

	(*pmyprint)();

	dlclose(library_handler);

	return 0;
}
