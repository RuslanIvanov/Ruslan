#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

unsigned int major = 240;
unsigned int minor = 0;
const char name[6] = "kbuf";
struct cdev * pcdev;
dev_t first_node;
#define COUNT_DEVICES 1

static ssize_t chkbuf_read(struct file * pfile, char __user * pbuf, size_t n, loff_t * poff)
{
	printk(KERN_INFO "read %s",name);
	return 0;
}

static ssize_t chkbuf_write(struct file *pfile, const char __user * pbuf, size_t n , loff_t * poff)
{
	printk(KERN_INFO "write %s",name);
	return 0;
}

static int chkbuf_open(struct inode *pinode, struct file * pfile)
{
	printk(KERN_INFO "open %s",name);
	return 0;
}

static int chkbuf_release(struct inode *pinode, struct file * pfile)
{
	printk(KERN_INFO "release %s",name);
	return 0;
}

static const struct file_operations chkbuf_fops = {
	.owner  = THIS_MODULE,
	.read = chkbuf_read,
	.write = chkbuf_write,
	.open = chkbuf_open,
	.release = chkbuf_release,
};

static  int chkbuf_init(void)
{
    int rez;
    
    printk(KERN_INFO "start ch module\n");

    rez  = register_chrdev(major, name,&chkbuf_fops);
    if(rez!=0)
    {
	printk(KERN_ALERT"Error register_chdev for %s:",name);
	if(rez == EINVAL)
            printk(KERN_ALERT"The specified number is not valid (> MAX_CHRDEV)");

	if(rez == EBUSY)
	    printk(KERN_ALERT"The major number is busy ");

	return rez;
    }
    
    first_node = MKDEV(major,minor);
    rez = register_chrdev_region(first_node,COUNT_DEVICES,name);

    if(rez!=0) 
    {
	printk(KERN_ALERT"Error register_chdev_region for %s:",name);
	return rez;
    }
    
    pcdev = cdev_alloc();
    cdev_init(pcdev,&chkbuf_fops);
    cdev_add(pcdev, first_node,COUNT_DEVICES);

//    pcdev->owner=THIS_MODULE;
//    cdev_add(pcdev, major, COUNT_DEVICES);

    return 0;
}

static void chkbuf_exit(void)
{
    unregister_chrdev_region(first_node, COUNT_DEVICES);
    cdev_del(pcdev);
    unregister_chrdev(major, name);

    printk(KERN_INFO "goodbye ch module\n");
}

module_init(chkbuf_init);
module_exit(chkbuf_exit);

MODULE_AUTHOR("Russl");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("module kbuf");
