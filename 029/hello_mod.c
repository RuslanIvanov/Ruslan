#include <linux/init.h>
#include <linux/module.h>
#include "module_init.h"

static int hello_init(void)
{
     printk("Hello module\n");
    return 0;
}

static void hello_exit(void)
{
    printk("Goodbye hello module\n");
}

void print_init()
{
	printk("i'm hello module\n");
}

module_init(hello_init);
module_exit(hello_exit);

EXPORT_SYMBOL(print_init);
