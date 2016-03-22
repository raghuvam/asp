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
#include <linux/device.h>

#define MYDEV_NAME "char_driver"

#define CDRV_IOC_MAGIC 'Z'

//data access directions
#define REGULAR 0
#define REVERSE 1

static size_t ramdisk_size = (16 * PAGE_SIZE);
static struct class *dev_class;
static struct cdev *my_cdev;
static dev_t first;
struct dev_area
{
 	char *ramdisk;
	dev_t dev;
	int dev_no;
	struct cdev *ncdev;
	struct semaphore sem;
	int seek_dir;
	struct dev_area* next;	
};


static struct dev_area *head = NULL;
static struct dev_area *tail = NULL;

static struct dev_area* create_dev_area(void)
{
	struct dev_area *temp = (struct dev_area*)kmalloc(sizeof(struct dev_area),GFP_KERNEL);
	if(head == NULL)
	{
		head = temp;
		tail = temp;
	}
	else
	{
		temp->next = NULL;
		tail->next = temp;
		tail = temp;
	}
	
	return tail;
}

static struct dev_area* get_dev_area(int dev_id)
{
	struct dev_area *temp = head;
	for(;dev_id >0;dev_id--)
		temp = temp->next;
	return temp;
}

static int free_device_list(void)
{
	while(head)
	{
		struct dev_area *temp = head->next;
		kfree(head->ramdisk);
		kfree(head);
		head = temp;
	}
	return 1;
}

static int mycdrv_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	int minor;
	struct dev_area *curr_dev;
	minor = iminor(inode);
	curr_dev = get_dev_area(minor);


	pr_info(" attempting to open device: %s:\n", MYDEV_NAME);

	 if(down_interruptible(&curr_dev->sem)) {
        printk(KERN_INFO " could not hold semaphore");
        return -1;
    }

	pr_info(" MAJOR number = %d, MINOR number = %d\n",
		imajor(inode), iminor(inode));
	counter++;
	
	// pass pointer to device to file private data
	file->private_data = curr_dev;

	pr_info(" successfully open  device: %s:\n\n", MYDEV_NAME);
	pr_info("I have been opened  %d times since being loaded\n", counter);
	pr_info("ref=%d\n", (int)module_refcount(THIS_MODULE));

	/* turn this on to inhibit seeking */
	/* file->f_mode = file->f_mode & ~FMODE_LSEEK; */

	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	struct dev_area *curr_dev = file->private_data;

	pr_info(" CLOSING device: %s: %d\n\n", MYDEV_NAME,curr_dev->dev_no);
	printk(KERN_INFO "Releasing semaphore");
    up(&curr_dev->sem);
	return 0;
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{	
	struct dev_area *curr_dev = file->private_data;

	int nbytes, maxbytes, bytes_to_do;
	
	if(curr_dev->seek_dir == REGULAR)
	{
		maxbytes = ramdisk_size - *ppos;
		bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
		
		if (bytes_to_do == 0)
			pr_info("Reached end of the device on a read");
		
		nbytes = bytes_to_do - copy_to_user(buf, curr_dev->ramdisk + *ppos, bytes_to_do);
		*ppos += nbytes;
		pr_info("\n Leaving the   READ function, nbytes=%d, pos=%d\n",
		nbytes, (int)*ppos);
		
		return nbytes;
	}

	// Read reverse
	else if(curr_dev->seek_dir == REVERSE)
	{
		maxbytes = *ppos;
		bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
		
		if (bytes_to_do == 0)
			pr_info("Reached start of the device for a reverse read");
		
		
		nbytes = bytes_to_do - copy_to_user(buf, curr_dev->ramdisk + *ppos-bytes_to_do, bytes_to_do);
		*ppos -= nbytes;
		pr_info("\n Leaving the   READ function, nbytes=%d, pos=%d\n",
		nbytes, (int)*ppos);
		
		return nbytes;
	}
	return -EINVAL;
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{	
	struct dev_area *curr_dev = file->private_data;

	int nbytes, maxbytes, bytes_to_do;
	maxbytes = ramdisk_size - *ppos;
	bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
	if (bytes_to_do == 0)
		pr_info("Reached end of the device on a write");
	nbytes =
	    bytes_to_do - copy_from_user(curr_dev->ramdisk + *ppos, buf, bytes_to_do);
	*ppos += nbytes;
	pr_info("\n Leaving the   WRITE function, nbytes=%d, pos=%d\n",
		nbytes, (int)*ppos);
	return nbytes;
}

static loff_t mycdrv_lseek(struct file *file, loff_t offset, int orig)
{
	//int *minor = file->private_data;
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


static int mycdrv_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct dev_area *curr_dev = file->private_data;

	int size, rc, direction;
	void __user *ioargp = (void __user *)arg;

	if (_IOC_TYPE(cmd) != CDRV_IOC_MAGIC) {
		pr_info(" got invalid case, CMD=%d\n", cmd);
		return -EINVAL;
	}

	direction = _IOC_DIR(cmd);
	size = _IOC_SIZE(cmd);

	switch (direction) {

	case _IOC_WRITE:
		pr_info( " Changing direction of operations\n");
		rc = copy_from_user(&curr_dev->seek_dir, ioargp, size);

		pr_info( " Direction = %d\n", curr_dev->seek_dir);
		return rc;
		break;

	default:
		pr_info(" got invalid case, CMD=%d\n", cmd);
		return -EINVAL;
	}

}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_open,
	.release = mycdrv_release,
	.llseek = mycdrv_lseek,
	.unlocked_ioctl = mycdrv_unlocked_ioctl
};

int NUM_DEVICES = 3;
module_param(NUM_DEVICES,int,0);

static int __init my_init(void)
{
	int i;

	// allocate devices with one major and NUM_DEVICES minor nums
	if (alloc_chrdev_region(&first,0, NUM_DEVICES, MYDEV_NAME) < 0)
	 {
		pr_err("failed to register character device region\n");
		return -1;
	}
	if (!(my_cdev = cdev_alloc())) 
	{
		pr_err("cdev_alloc() failed\n");
		unregister_chrdev_region(first, NUM_DEVICES);
		return -1;
	}

	

	if ((dev_class = class_create(THIS_MODULE, MYDEV_NAME)) == NULL)    //$ls /sys/class
    {
        unregister_chrdev_region(first, 1);
        return -1;
    }

	for(i=0;i< NUM_DEVICES;i++)
	{
		

    	if (device_create(dev_class, NULL, MKDEV(MAJOR(first), MINOR(first) + i), NULL, "%s%d",MYDEV_NAME, i)== NULL) //$ls /dev/
    	{
    		i--;
        	while(i>=0)
        	{
        		device_destroy(dev_class,MKDEV(MAJOR(first),MINOR(first) +i));
        		i--;
        	}
        	class_destroy(dev_class);

        	unregister_chrdev_region(first, 1);
        	return -1;
    	}

	}
	

	cdev_init(my_cdev, &mycdrv_fops);
	

	// create structs for each device
	for(i = 0 ; i < NUM_DEVICES;i++)
	{	
		struct dev_area *curr_dev = create_dev_area();
		curr_dev->ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
		curr_dev->dev_no = i;
		curr_dev->dev = MKDEV(MAJOR(first), MINOR(first) + i);
		curr_dev->ncdev = my_cdev;
		curr_dev->seek_dir= REGULAR;
		//init semaphore
		sema_init(&curr_dev->sem,1);

	}
		
	if (cdev_add(my_cdev, first, NUM_DEVICES) < 0) {
		pr_err("cdev_add() failed\n");
		cdev_del(my_cdev);
		unregister_chrdev_region(first, NUM_DEVICES);
		
		// call free device memory
		free_device_list();
		return -1;
	}

	pr_info("\nSucceeded in registering character devices %s\n", MYDEV_NAME);
	return 0;
}

static void __exit my_exit(void)
{
	int j = 0;
	if (my_cdev)
		cdev_del(my_cdev);

	
	while(j< NUM_DEVICES)
   	{
      	device_destroy(dev_class,MKDEV(MAJOR(first),MINOR(first) +j));
       	j++;
   	}

  	class_destroy(dev_class);
	unregister_chrdev_region(first, NUM_DEVICES);
	
	// free devices memory linkedlist
	free_device_list();
	
	pr_info("\ndevice unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_04/lab3_seek.c");
MODULE_LICENSE("GPL v2");
