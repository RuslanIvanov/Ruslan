#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include "ioctl_kbuf.h"

unsigned int major = 701;
unsigned int minor = 0;
const char name[6] = "kbuf";
struct cdev * pcdev;
dev_t first_node;
int dev_open = 0;

#define COUNT_DEVICES 1
#define KBUF_BUF 200 

unsigned char *pbuf=0;
unsigned int pids[KBUF_BUF];

static long chkbuf_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	int err;
	int retval; 

	retval = -1;// 0 - команда  выпоненена, -1 или  <0 - команда не выполнена  - error

	/* проверить тип и номер битовых полей и не декодировать
	* неверные команды: вернуть ENOTTY (неверный ioctl) перед access_ok( )
	*/

	//printk(KERN_INFO "MOD_PSR: psr_ioctl");
	if (_IOC_TYPE(cmd) != PSR_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > PSR_IOC_MAXNR) return -ENOTTY;

	/*
	 * направление является битовой маской и VERIFY_WRITE отлавливает передачи R/W
	 * `направление' является ориентированным на пользователя, в то время как
	 * access_ok является ориентированным на ядро, так что концепции "чтение" и
	 * "запись" являются обратными
	 */

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));


	if (err) return -EFAULT;

	switch(cmd)
	{

        case KBUF_IO_PIDS:
        {
		return -EFAULT;
    		
	    	retval = 0;
	}
	break;
	case KBUF_IOCG_STATISTIC:
	{
		int rez; rez=0;
            	printk(KERN_INFO "KBUF_IOCG_STATISTIC:");
            	rez=copy_to_user((int __user *)arg, (char*)&pids[0], sizeof(pids));

	    	if(rez)
            	{
               		printk(KERN_ERR "KBUF_IOCG_STATISTIC: error copy_to_user witch IOCTL");
               		return -EFAULT;
            	}

	    	retval = 0;
	}
	break;
	default:  /* redundant, as cmd was checked against MAXNR */
	return -ENOTTY;
	}

	return retval;

}

static loff_t chkbuf_lseek (struct file* filp, loff_t off,int whence)
{
	
	loff_t newpos; 

	switch(whence) 
	{ 
	case 0: /* SEEK_SET */ //0,1,2
		newpos = off; 
	break; 
	case 1: /* SEEK_CUR */ 
		newpos = filp->f_pos + off; 
	break; 
	case 2: /* SEEK_END */ 
		//newpos = dev->size + off; 
		newpos=2;
	default: /* не может произойти */ 
	return -EINVAL; 
	} 

	if (newpos < 0) return -EINVAL; 
		filp->f_pos = newpos; 
	return newpos; 
}

static ssize_t chkbuf_read(struct file * pfile, char __user * pbuf, size_t n, loff_t * poff)
{
	int rez;
	loff_t pos;

	printk(KERN_INFO "read %s",name);

 	pos = /*file->f_pos;*/=*poff
	if (pos >= KBUF_BUF) return -EFAULT;
	
	if (pos + count > KBUF_BUF)
		count = KBUF_BUF - pos;
	else count = n;


	rez=copy_to_user((char __user *)arg, (char*)&pbuf[0], n);	
        if(rez)
        {
               	printk(KERN_ERR " chkbuf_read: error read buf!");
               	return -EFAULT;
       	}

	return 0;
}

static ssize_t chkbuf_write(struct file *pfile, const char __user * pbuf, size_t n , loff_t * poff)
{
	int pos; 
	pos = 0; 
	int count;
	count=0;

	printk(KERN_INFO" chkbuf_write %s",name);

 	pos = /*file->f_pos;*/=*poff
	if (pos >= KBUF_BUF) return -EFAULT;
	
	if (pos + count > KBUF_BUF)
		count = KBUF_BUF - pos;
	else count = n;

	rez=copy_from_user((char*)&pbuf[KBUF_BUF-count],(int __user *)arg,count);
      	if(rez)
       	{
		printk(KERN_ERR" chkbuf_write: error write buf!");
       		return -EINVAL;
	}

	*poff += count;

	return count;
}

static int chkbuf_open(struct inode *pinode, struct file * pfile)
{
	printk(KERN_INFO "open %s",name);

	if(dev_open == 0)
	{
		dev_open++;
		pbuf = kmalloc(KBUF_BUF+1,GFP_KERNEL);
		if(pbuf == NULL) return -ENOMEM;

		struct scull_dev *dev; /* информация об устройстве */
		dev = container_of(inode->i_cdev, struct scull_dev, cdev);
		filp->private_data = dev; /* для других методов */
	}	

	if(dev_open > 0)
	{
		printk(KERN_ALERT "Error: /dev/chkbuf already open!");
	}

	return 0;
}

static int chkbuf_release(struct inode *pinode, struct file * pfile)
{
	printk(KERN_INFO "release %s",name);

	if(dev_open > 0)
	{
		dev_open--;
	}

	if(pbuf)
	{
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
