//#define SHARED_LIB
#include "print.h"
#include <dlfcn.h>

int main()
{
    printf("\n\tHello world\n");
#ifdef SHARED_LIB
    myprint();
#else
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

#endif
}
