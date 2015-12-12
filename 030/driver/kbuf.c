#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include "ioctl_kbuf.h"

unsigned int major = 701;
unsigned int minor = 0;
const char name[6] = "kbuf";
struct cdev * pcdev;
dev_t first_node;
int dev_open = 0;

#define COUNT_DEVICES 1
#define KBUF_BUF 2000 
#define EOF 0

struct STATISTIC_RW statistic;

/*struct chkbuf_dev
{
    int size;
    char name[6];
    struct semaphore sem;
    struct cdev cdev;
};

struct chkbuf_dev mychdev;
mychdev.size = KBUF_BUF;*/

unsigned char *pbuf=0;

struct PID_INFO pid_info;

struct pid *pid_struct;
struct task_struct *ptask;

loff_t posW;
loff_t posR;

unsigned int numIrq;
int dev;
//typedef irqreturn_t (*irq_handler_t)( int, void* );
static irqreturn_t inter_handler ( int irq, void *dev )
{
	int mydev;
	mydev = *((int*)dev);
	if( mydev == 1982 )
	{
		numIrq++;
	   	return IRQ_HANDLED;
	}

	return IRQ_NONE;
}


static long chkbuf_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	int err;
	int retval;
	err = 0; retval = 0;

	retval = -1;// 0 - команда  выпоненена, -1 или  <0 - команда не выполнена  - error

	printk(KERN_INFO" chkbuf: ioctl"); 

	if (_IOC_TYPE(cmd) != KBUF_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > KBUF_IOC_MAXNR) return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

	if (err) return -EFAULT;

	switch(cmd)
	{

        case KBUF_IOCX_IO_PID:
        {
		retval = __get_user(pid_info.pid, (int __user *)arg);

		printk(KERN_INFO "KBUF_IOCX_IO_PID: get pid %d", pid_info.pid);

		if(pid_info.pid == 0 || retval != 0)
		{ retval = -EFAULT; }
		else {

			int len=0;
			pid_struct = find_vpid(pid_info.pid);
			ptask = pid_task(pid_struct,PIDTYPE_PID);

			len = sprintf(pid_info.buf,"task is '%s', pid = %d, parent is '%s'",ptask->comm, ptask->pid, ptask->parent->comm);
			retval=copy_to_user((int __user *)arg, (char*)&pid_info, sizeof(struct PID_INFO));
              		if(retval)
            		{
		                printk(KERN_ERR "KBUF_IOCX_IO_PID: error copy_to_user witch IOCTL");
                		return -EFAULT;
            		}

			//printk(KERN_INFO  " information about process %d: %s",pid_info.pid, pid_info.buf);
			printk(KERN_INFO  " current task is %s [%d] parent %s", current->comm, current->pid, current->parent->comm );

			retval = 0; 
			}
	}
	break;
	case KBUF_IOCG_STATISTIC:
	{
		int rez; rez=0;
		printk(KERN_INFO "KBUF_IOCG_STATISTIC");
		rez=copy_to_user((int __user *)arg, (char*)&statistic, sizeof(struct STATISTIC_RW));

	    	if(rez)
            	{
               		printk(KERN_ERR "KBUF_IOCG_STATISTIC: error copy_to_user witch IOCTL");
               		return -EFAULT;
            	}

		printk(KERN_INFO  " statistic for device %s: [read %d, write %d]",name,statistic.cr,statistic.cw);

	    	retval = 0;
	}
	break;
	case KBUF_IOCG_GETNUMIRQ:
		retval = __put_user(numIrq, (int __user *)arg);
		if(retval!=0) {printk(KERN_ERR "KBUF_IOCG_GETNUMIRQ: error get number irq witch IOCTL");}
	break;
	default: 
	return -ENOTTY;
	}

	return retval;

}

static loff_t chkbuf_lseek (struct file* filp, loff_t off,int whence)
{

	loff_t newpos;
	newpos = 0;

	switch(whence)
	{
	case 0: /* SEEK_SET */
		newpos = off;
	break;
	case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
	break;
	case 2: /* SEEK_END */
		//newpos = dev->size + off;
		newpos=KBUF_BUF+off;
	default: /* не может произойти */
	return -EINVAL;
	}

	if (newpos < 0) return -EINVAL;
		filp->f_pos = newpos;
	return newpos;
}

static ssize_t chkbuf_read(struct file * pfile, char __user * pbufu, size_t n, loff_t * poff)
{
	int rez;
	int pos;
	int currentN;

	pos = *poff; //file->f_pos==*poff

	printk(KERN_INFO " chkbuf_read %s, pos %d",name, pos);

	if(pos >= KBUF_BUF) return EOF;

	if(pos >= posW) return EOF;
	if(posW > n)
	{
		printk(KERN_ERR " chkbuf_read: small reception buffer for %s",name);
		return -EFAULT; //маленький приемный буффер
	}

	//коррекция n:
	currentN = posW - pos;
//	if(n < pos){ currentN = n - pos; n = n - currentN; }

	rez = currentN - copy_to_user((char __user *)pbufu, (char*)&pbuf[pos], currentN); //

//	rez = n - copy_to_user((char __user *)pbufu, (char*)&pbuf[pos], n);
        if(rez==0)
        {
               	printk(KERN_ERR " chkbuf_read: error read buf!");
               	return -EFAULT;
	}

	*poff += rez;
	posR=*poff;
	statistic.cr++;
	return rez; //was readed
}

static ssize_t chkbuf_write(struct file *pfile, const char __user * pbufu, size_t n , loff_t * poff)
{
	int pos; int rez;
	pos = 0; 

	pos =*poff;
	printk(KERN_INFO" chkbuf_write %s, pos %d",name, pos);

	if (pos >= KBUF_BUF) return EOF;

	rez = n - copy_from_user((char*)&pbuf[pos],(int __user *)pbufu,n);
	if(rez==0)
	{
		printk(KERN_ERR" chkbuf_write: error write buf!");
		return -EINVAL;
	}

	*poff += rez;
	posW = *poff;
	statistic.cw++;
	return rez; // was writted
}

static int chkbuf_open(struct inode *pinode, struct file * pfile)
{
	int rez; 

	printk(KERN_INFO " chkbuf_open %s",name);
	
	posW=0; posR=0;
	rez=0; dev=0;

	if(dev_open > 0)
	{
		printk(KERN_ERR "Error: /dev/chkbuf already open!");
		return -EINVAL;
	}

	if(dev_open == 0)
	{

		dev=1982;
		rez = request_irq(19, inter_handler, IRQF_SHARED, "IRQ_KBUF", &dev);
		if(rez != 0)
		{
			printk(KERN_ERR " Error  request_irq N 19 for /dev/chkbuf");
			return rez;
		}

		dev_open++;
		pbuf = kmalloc(KBUF_BUF,GFP_KERNEL);
		if(pbuf == NULL)
		{ 
			printk(KERN_ERR " Error kmalloc for /dev/chkbuf");
			return -ENOMEM;
		}

//		struct chkbuf_dev *dev; 
//		dev = container_of(inode->i_cdev, struct chkbuf_dev, cdev);
//		filp->private_data = dev; 

	}

	return 0;
}

static int chkbuf_release(struct inode *pinode, struct file * pfile)
{
	printk(KERN_INFO " chkbuf_release %s",name);

	if(dev_open > 0)
	{
		dev_open--;
	}

	if(pbuf)
	{
		free_irq(19, &dev);
		kfree(pbuf);
		pbuf=0;
	}
	
	return 0;
}

static const struct file_operations chkbuf_fops = {
	.owner  = THIS_MODULE,
	.read = chkbuf_read,
	.write = chkbuf_write,
	.open = chkbuf_open,
	.release = chkbuf_release,
	.llseek = chkbuf_lseek,
	.unlocked_ioctl = chkbuf_ioctl,
};

static  int chkbuf_init(void)
{
    	int rez;
    	printk(KERN_INFO "start ch module\n");

    	//Старый способ регистрации
    	/*
    	rez  = register_chrdev(major, name ,&chkbuf_fops);

    	if(rez!=0)
    	{
		printk(KERN_ALERT"Error register_chdev for %s:",name);
		if(rez == EINVAL)
	            printk(KERN_ALERT"The specified number is not valid (> MAX_CHRDEV)");

		if(rez == EBUSY)
		    printk(KERN_ALERT"The major number is busy ");
	
		return rez;
	}*/
	
	//новый способ регитсрации
	first_node = MKDEV(major,minor);
	rez = register_chrdev_region(first_node,COUNT_DEVICES,name);// получение номера(ов) символьного устройства

	if(rez!=0) 
	{
		printk(KERN_ALERT"Error register_chdev_region for %s code %d",name,rez);
		unregister_chrdev_region(first_node, COUNT_DEVICES);
		return rez;
	}
    
	pcdev = cdev_alloc();

	cdev_init(pcdev,&chkbuf_fops);
	pcdev->owner=THIS_MODULE;
	pcdev->ops=&chkbuf_fops;
	rez = cdev_add(pcdev, first_node,COUNT_DEVICES);

	if ( rez < 0 )
	{
		unregister_chrdev_region(first_node, COUNT_DEVICES);	
	}

    return 0;
}

static void chkbuf_exit(void)
{
	unregister_chrdev_region(first_node, COUNT_DEVICES);
	cdev_del(pcdev);
	//unregister_chrdev(major, name);

	printk(KERN_INFO "goodbye ch module\n");
}

module_init(chkbuf_init);
module_exit(chkbuf_exit);

MODULE_AUTHOR("Russl");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("module kbuf");
