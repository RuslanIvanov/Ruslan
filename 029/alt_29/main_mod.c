#include <linux/init.h>
#include <linux/module.h>
#include "module_init.h"

static int main_init(void)
{
    printk("main module\n");
    print_init();
    return 0;
}

static void main_exit(void)
{
    printk("Goodbye, main module\n");
}

module_init(main_init);
module_exit(main_exit);

MODULE_AUTHOR("Russl");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Main module call hello func");