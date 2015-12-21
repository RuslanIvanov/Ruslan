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
#include <linux/wait.h>
#include <linux/poll.h>

#include "ioctl_kbuf.h"

int countDev = 0;

module_param(countDev,int,0);

unsigned int major = 701;
unsigned int minor = 0;
const char name[6] = "kbuf";

dev_t first_node;
int dev_open = 0;

int COUNT_DEVICES  = 1;
#define KBUF_BUF 200 
#define EOF 0

struct STATISTIC_RW statistic;

struct chkbuf_dev
{
    	int size;
	unsigned char buf[KBUF_BUF];
    	struct mutex mutex;
    	struct cdev cdev;
	loff_t posW;
	loff_t posR;
};

struct chkbuf_dev* pcdev;

struct PID_INFO pid_info;

struct pid *pid_struct;
struct task_struct *ptask;

unsigned int numIrq;
int dev;//

static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;

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

			//для текущего процесса current:
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

static loff_t chkbuf_lseek (struct file* pfile, loff_t off,int whence)
{
	struct chkbuf_dev *dev;
	loff_t newpos;

	newpos = 0;
	dev = pfile->private_data; 

	switch(whence)
	{
		case 0: /* SEEK_SET */
			newpos = off;
		break;
		case 1: /* SEEK_CUR */
			newpos = pfile->f_pos + off;
		break;
		case 2: /* SEEK_END */
			newpos=dev->size+off;
		default: /* не может произойти */
		return -EINVAL;
	}

	if (newpos < 0) return -EINVAL;
		pfile->f_pos = newpos;
	return newpos;
}

static ssize_t chkbuf_read(struct file * pfile, char __user * pbufu, size_t n, loff_t * poff)
{
	int rez;
	int pos;
	struct chkbuf_dev *dev;

	dev = pfile->private_data; 

	if((pfile->f_flags & O_ACCMODE)!=O_NONBLOCK)
	{
		wait_event_interruptible(wq, flag != 0);
        	flag = 0;
	}

	mutex_lock_interruptible(&dev->mutex);

	pos = *poff; //file->f_pos==*poff

	printk(KERN_INFO " chkbuf_read %s, pos %d\n",name, pos);

	if(pos >= dev->size) 
	{
		mutex_unlock(&dev->mutex);
		return EOF;
	}

	if(n>dev->posW)
	{
		n=dev->posW;
	}

	if(pos>=dev->posW) 
	{
		mutex_unlock(&dev->mutex);
		return EOF;
	}

	rez = n - copy_to_user((char __user *)pbufu, (char*)&dev->buf[pos], n);
        if(rez==0)
        {
		mutex_unlock(&dev->mutex);
               	printk(KERN_ERR " chkbuf_read: error read buf!\n");
               	return -EFAULT;
	}

	*poff += rez;

	statistic.cr++;

	mutex_unlock(&dev->mutex);

	printk(KERN_INFO " chkbuf_read current pos %d\n",(int)(*poff));

	return rez; //was readed
}

static ssize_t chkbuf_write(struct file *pfile, const char __user * pbufu, size_t n , loff_t * poff)
{
	int pos; 
	int rez;
 	struct chkbuf_dev *dev;

	pos = 0;
	dev = pfile->private_data; 

	if((pfile->f_flags & O_ACCMODE)!=O_NONBLOCK)
	{
		flag = 1;
        	wake_up_interruptible(&wq);
	}

	mutex_lock_interruptible(&dev->mutex);

	pos =*poff;
	printk(KERN_INFO" chkbuf_write %s, pos %d\n",name, pos);

	if (pos >= dev->size) 
	{
		mutex_unlock(&dev->mutex);
		return EOF;
	}

	if(n > dev->size)
	{
		mutex_unlock(&dev->mutex);
		n = KBUF_BUF;
		printk(KERN_ERR" chkbuf_write: error (n > dev->size)!\n");

	}

	rez = n - copy_from_user((char*)&dev->buf[pos],(int __user *)pbufu,n);
	if(rez==0)
	{
		mutex_unlock(&dev->mutex);
		printk(KERN_ERR" chkbuf_write: error write buf!\n");
		return -EINVAL;
	}

	*poff += rez;
	dev->posW = *poff;

	statistic.cw++;

	mutex_unlock(&dev->mutex);

	printk(KERN_INFO " chkbuf_write current pos %d\n",(int)(*poff));

	return rez; // was writted
}

static unsigned int chkbuf_poll(struct file *pfile, poll_table *wait)
{
	unsigned int mask;
	struct chkbuf_dev *dev;

	dev = pfile->private_data; 
	mask = 0;

	poll_wait(pfile, &wq, wait);

	mutex_lock_interruptible(&dev->mutex);

	if (pfile->f_pos != dev->posW)
		mask |= POLLIN | POLLRDNORM; //чтение 

	mutex_unlock(&dev->mutex);

	//обработать конец файла. вернуть POLLHUP	

	return mask;
}

static int chkbuf_open(struct inode *pinode, struct file * pfile)
{
        struct  chkbuf_dev  *dev;

 
	if(dev_open > 0 && countDev==0)
	{
		printk(KERN_ERR "Error: /dev/chkbuf already open!\n");
		return -EINVAL;
	}

	if(dev_open == 0 || countDev>0)
	{
		dev_open++;
	}

	dev = container_of(pinode->i_cdev,struct chkbuf_dev,cdev);

        pfile->private_data = dev;

	pfile->f_pos=0;

	printk(KERN_INFO " chkbuf_open: count devices %d ('=0' - for one, '>0' - for more)\n",countDev);

        return 0;
}

static int chkbuf_release(struct inode *pinode, struct file * pfile)
{
        struct chkbuf_dev *dev;
        dev = pfile->private_data;

	printk(KERN_INFO " chkbuf_release %s\n",name);

	if((pfile->f_flags & O_ACCMODE)!=O_NONBLOCK)
        {
                flag = 1;
                wake_up_interruptible(&wq);
        }


	if(dev_open > 0)
	{
		dev_open--;
	}

	dev->posW=0;

	if(dev_open==0)
	{
		printk(KERN_INFO " chkbuf_release close all devices\n");
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
	.poll =	chkbuf_poll,
	.unlocked_ioctl = chkbuf_ioctl,
};

static  int chkbuf_init(void)
{
    	int rez;
    	printk(KERN_INFO " start kbuf module\n");

	COUNT_DEVICES = countDev+1;
	printk(KERN_INFO " COUNT_DEVICES %d\n",COUNT_DEVICES);

	//новый способ регисtрации
	first_node = MKDEV(major,minor);
	rez = register_chrdev_region(first_node,COUNT_DEVICES,name);// получение номера(ов) символьного устройства

	if(rez!=0) 
	{
		printk(KERN_ERR" Error register_chdev_region for %s code %d\n",name,rez);
		unregister_chrdev_region(first_node, COUNT_DEVICES);
		return rez;
	}

	//pcdev = cdev_alloc();

	pcdev = kmalloc(sizeof (struct chkbuf_dev), GFP_KERNEL);

	if(pcdev == NULL)
        {
               printk(KERN_ERR " Error kmalloc for /dev/chkbuf\n");
               return -ENOMEM;
        }

        pcdev->size = KBUF_BUF;
        mutex_init(&(pcdev->mutex));  
        pcdev->posW=0;
        pcdev->posR=0;

	cdev_init(&pcdev->cdev,&chkbuf_fops);
	pcdev->cdev.owner=THIS_MODULE;
	pcdev->cdev.ops=&chkbuf_fops;
	rez = cdev_add(&pcdev->cdev, first_node,COUNT_DEVICES);

	if ( rez < 0 )
	{
		unregister_chrdev_region(first_node, COUNT_DEVICES);
		return rez;
	}

	dev=1982;
	rez = request_irq(19, inter_handler, IRQF_SHARED, "IRQ_KBUF", &dev);
	if(rez != 0)
	{
		printk(KERN_ERR " Error  request_irq N 19 for /dev/chkbuf\n");
		return rez;
	}

    	return 0;
}

static void chkbuf_exit(void)
{
	unregister_chrdev_region(first_node, COUNT_DEVICES);
	cdev_del(&pcdev->cdev);

	if(pcdev)
        {
                kfree(pcdev);
                pcdev=0;
		printk(KERN_INFO " kfree for kbuf)\n");
        }

	free_irq(19, &dev);
	printk(KERN_INFO " free_irq for kbuf \n");

	printk(KERN_INFO " goodbye, kbuf module\n");
}

module_init(chkbuf_init);
module_exit(chkbuf_exit);

MODULE_AUTHOR("Russl");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("module kbuf");
