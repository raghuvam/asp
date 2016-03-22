/* **************** LDD:2.0 s_04/lab3_seek.c **************** */
/*
Author: Sai Raghu Vamsi Anumula
UFID: 49939544 
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>

#define MYDEV_NAME "char_driver		"



static char *ramdisk[10];
static size_t ramdisk_size = (16 * PAGE_SIZE);
static dev_t first;
static unsigned int count = 1;

static struct cdev *my_cdev;
static struct semaphore sem;



static int mycdrv_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	pr_info(" attempting to open device: %s:\n", MYDEV_NAME);

	 if(down_interruptible(&sem)) {
        printk(KERN_INFO " could not hold semaphore");
        return -1;
    }

	pr_info(" MAJOR number = %d, MINOR number = %d\n",
		imajor(inode), iminor(inode));
	counter++;
	
	int *minor = kmalloc(sizeof(int),GFP_KERNEL);
	*minor = iminor(inode);

	file->private_data = minor;
	pr_info(" successfully open  device: %s:\n\n", MYDEV_NAME);
	pr_info("I have been opened  %d times since being loaded\n", counter);
	pr_info("ref=%d\n", (int)module_refcount(THIS_MODULE));

	/* turn this on to inhibit seeking */
	/* file->f_mode = file->f_mode & ~FMODE_LSEEK; */

	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
	printk(KERN_INFO "Releasing semaphore");
	kfree(file->private_data);
    up(&sem);
	return 0;
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{	
	int *minor = file->private_data;

	int nbytes, maxbytes, bytes_to_do;
	maxbytes = ramdisk_size - *ppos;
	bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
	if (bytes_to_do == 0)
		pr_info("Reached end of the device on a read");
	nbytes = bytes_to_do - copy_to_user(buf, ramdisk[*minor] + *ppos, bytes_to_do);
	*ppos += nbytes;
	pr_info("\n Leaving the   READ function, nbytes=%d, pos=%d\n",
		nbytes, (int)*ppos);
	return nbytes;
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{	
	int *minor = file->private_data;

	int nbytes, maxbytes, bytes_to_do;
	maxbytes = ramdisk_size - *ppos;
	bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
	if (bytes_to_do == 0)
		pr_info("Reached end of the device on a write");
	nbytes =
	    bytes_to_do - copy_from_user(ramdisk[*minor] + *ppos, buf, bytes_to_do);
	*ppos += nbytes;
	pr_info("\n Leaving the   WRITE function, nbytes=%d, pos=%d\n",
		nbytes, (int)*ppos);
	return nbytes;
}

static loff_t mycdrv_lseek(struct file *file, loff_t offset, int orig)
{
	int *minor = file->private_data;
	loff_t testpos;
	switch (orig) {
	case SEEK_SET:
		testpos = offset;
		break;
	case SEEK_CUR:
		testpos = file->f_pos + offset;
		break;
	case SEEK_END:
		testpos = ramdisk_size + offset;
		break;
	default:
		return -EINVAL;
	}
	testpos = testpos < ramdisk_size ? testpos : ramdisk_size;
	testpos = testpos >= 0 ? testpos : 0;
	file->f_pos = testpos;
	pr_info("Seeking to pos=%ld\n", (long)testpos);
	return testpos;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_open,
	.release = mycdrv_release,
	.llseek = mycdrv_lseek
};

int NUM_DEVICES = 3;
module_param(NUM_DEVICES,int,0);

static int __init my_init(void)
{


	if (alloc_chrdev_region(&first,0, NUM_DEVICES, MYDEV_NAME) < 0) {
		pr_err("failed to register character device region\n");
		return -1;
	}
	if (!(my_cdev = cdev_alloc())) {
		pr_err("cdev_alloc() failed\n");
		unregister_chrdev_region(first, NUM_DEVICES);
		return -1;
	}
	cdev_init(my_cdev, &mycdrv_fops);
	int i=0;

	for(i = 0 ; i < NUM_DEVICES;i++)
		ramdisk[i] = kmalloc(ramdisk_size, GFP_KERNEL);	
	
	//init semaphore
	sema_init(&sem,1);


	if (cdev_add(my_cdev, first, NUM_DEVICES) < 0) {
		pr_err("cdev_add() failed\n");
		cdev_del(my_cdev);
		unregister_chrdev_region(first, NUM_DEVICES);
		
		for(i = 0 ; i < NUM_DEVICES;i++)
			kfree(ramdisk[i]);

		return -1;
	}

	pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
	return 0;
}

static void __exit my_exit(void)
{
	if (my_cdev)
		cdev_del(my_cdev);
	unregister_chrdev_region(first, NUM_DEVICES);
	
	int i=0;
	for(i = 0 ; i < NUM_DEVICES;i++)
			kfree(ramdisk[i]);
	
	pr_info("\ndevice unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_04/lab3_seek.c");
MODULE_LICENSE("GPL v2");
