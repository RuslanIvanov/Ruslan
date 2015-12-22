#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/etherdevice.h>
#include <linux/bitops.h>
#include <linux/device.h>

#include "ioctl_netkbuf.h"

int countDev = 0;

module_param(countDev,int,0);

unsigned int major = 702;
unsigned int minor = 0;
const char name[20] = "netkbuf";

dev_t first_node;

int COUNT_DEVICES  = 1;
#define KBUF_BUF 200 

struct STATISTIC_RW statistic;

struct netkbuf_dev
{
	unsigned char buf[KBUF_BUF];
    	struct net_device *dev;
};

struct net_device* pnetdev;
struct cdev * pcdev;
struct net_device_stats *statusNet;

static ssize_t chkbuf_read(struct file * pfile, char __user * pbufu, size_t n, loff_t * poff)
{
	int rez;
	
	rez = 0;
	return rez; //was readed
}

static ssize_t chkbuf_write(struct file *pfile, const char __user * pbufu, size_t n , loff_t * poff)
{
	 
	int rez;
	rez=0;
 	
	return rez; // was writted
}

static int chkbuf_open(struct inode *pinode, struct file * pfile)
{
    	printk(KERN_INFO " chkbuf_open: count devices %d ('=0' - for one, '>0' - for more)\n",countDev);

        return 0;
}

static int chkbuf_release(struct inode *pinode, struct file * pfile)
{
	printk(KERN_INFO " chkbuf_release\n");
       	return 0;
}

static int  net_init(struct net_device *dev)
{
	printk(KERN_INFO " net_init\n");
	return 0;
}

void net_uninit (struct net_device *dev)
{
	printk(KERN_INFO " net_uninit\n");
}

int  net_open (struct net_device *dev)
{
	printk(KERN_INFO " net_open\n");	
	return 0;
}

int net_stop (struct net_device *dev)
{
	printk(KERN_INFO " net_stop\n");
	return 0;
}

netdev_tx_t net_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	printk(KERN_INFO " net_start_xmit\n");

	//Returns NETDEV_TX_OK

	return NETDEV_TX_BUSY;
}

void net_tx_timeout(struct net_device *dev)
{
	
}

struct net_device_stats* net_get_stats (struct net_device *dev)
{
	
	statusNet->rx_packets=0;
	statusNet->tx_packets=0;
	statusNet->rx_bytes=0;
	statusNet->tx_bytes=0;
	statusNet->rx_errors=0;
	statusNet->tx_errors=0;

	return statusNet;
}

static const struct file_operations chkbuf_fops = {
	.owner  = THIS_MODULE,
	.read = chkbuf_read,
	.write = chkbuf_write,
	.open = chkbuf_open,
	.release = chkbuf_release,

};

static const struct net_device_ops net_fops = {
	//http://lxr.free-electrons.com/source/include/linux/netdevice.h
	.ndo_init = net_init,
	.ndo_uninit = net_uninit,
	.ndo_open = net_open,
	.ndo_stop = net_stop,
	.ndo_start_xmit = net_start_xmit,
	.ndo_tx_timeout = net_tx_timeout,
	.ndo_get_stats = net_get_stats,

};

static void netdevice_init (struct net_device * dev)
{
	int timeout; 
	
	struct netkbuf_dev *priv; 

	priv = netdev_priv(dev);

	ether_setup(dev);
	timeout = 10;
//	dev->watchdoc_timeo = timeout;
	dev->netdev_ops = &net_fops;
	dev->flags |= IFF_NOARP; 

	printk(KERN_INFO " netdevice_init\n");
}

static struct net_device* netdevice_create(const char* name)
{
	struct net_device* dev;
	dev = alloc_netdev(sizeof(struct netkbuf_dev),name,NET_NAME_UNKNOWN,netdevice_init);

	return dev;
}

static  int chkbuf_init(void)
{
    	int rez;
    	printk(KERN_INFO " start netkbuf module\n");

    	//COUNT_DEVICES = countDev+1;
	printk(KERN_INFO "COUNT_DEVICES %d\n",COUNT_DEVICES);
	
	first_node = MKDEV(major,minor);
	rez = register_chrdev_region(first_node,COUNT_DEVICES,name);// получение номера(ов) символьного устройства

	if(rez!=0)
	{
		printk(KERN_ERR" Error register_chdev_region for %s code %d\n",name,rez);
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
		return rez;
	}
	
	//net registration
	
	pnetdev = netdevice_create("netkbuf");
	if(pnetdev == NULL)
        { 
               printk(KERN_ERR " Error kmalloc for netdevice_create\n");
	       unregister_chrdev_region(first_node, COUNT_DEVICES);
	       cdev_del(pcdev);
               return -ENOMEM;
        }

	rez=register_netdev(pnetdev);

	if(rez < 0)
	{
		printk(KERN_ERR " Error register_netdev\n");
		unregister_chrdev_region(first_node, COUNT_DEVICES);
		cdev_del(pcdev);
		free_netdev(pnetdev);

//		if(pnetdev){kfree(pnetdev);pnetdev=0;}

		return rez;
	}
	
    	return 0;
}

static void chkbuf_exit(void)
{
	unregister_chrdev_region(first_node, COUNT_DEVICES);
	cdev_del(pcdev);
	unregister_netdev(pnetdev);
	free_netdev(pnetdev);
	
	/*if(pnetdev)
        {
                kfree(pnetdev);
                pnetdev=0;
		printk(KERN_INFO " kfree for pnetdev)\n");
        }*/

/*
	if(pcdev)
        {
                kfree(pcdev);
                pcdev=0;
		printk(KERN_INFO " kfree for pcdev)\n");
        }*/

	printk(KERN_INFO " goodbye, kbuf module\n");
}

module_init(chkbuf_init);
module_exit(chkbuf_exit);

MODULE_AUTHOR("Russl");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("module kbuf");
