#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fcntl.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gianluca Galvagni");

// Global variables
int my_minor = 0;
int my_major = 0;
int my_mem_size = 255;

// Device structure
module_param(my_minor, int, S_IRUGO);
module_param(my_major, int, S_IRUGO);
module_param(my_mem_size, int, S_IRUGO);

struct my_dev {
    char *data;
    struct semaphore sem;
    struct cdev cdev;
    int mem_size;
};

struct my_dev *my_devices;

// ---------------------------file operations---------------------------

// Open
static int my_open(struct inode *inode, struct file *filp)
{
    struct my_dev *dev;

    dev = container_of(inode->i_cdev, struct my_dev, cdev);
    filp->private_data = dev;

    return 0;
}

// Read from device
static int my_release(struct inode *inode, struct file *filp)
{
    return 0;
}

// Write to device
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // Get device
    struct my_dev *dev = filp->private_data;
    ssize_t retval = 0;

    // Check if the device is writable
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    if (count > dev->mem_size)
        count = dev->mem_size;

    // Copy data from user space
    if (copy_from_user(dev->data, buf, count)) {
        retval = -EFAULT;
        goto out;
    }

    // print data
    fprintk(KERN_INFO, "My_write: %s", dev->data);

    // Update file position
    *f_pos += count;
    retval = count;

// Exit
out:
    up(&dev->sem);
    return retval;
}

struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .write = my_write,
};

// ---------------------------proc file operations---------------------------
void my_cleanup_module(void)
{
    int i;
    dev_t devno = MKDEV(my_major, my_minor);

    // Remove proc file
    if (my_devices) {
        for (i = 0; i < 1; i++) {
            cdev_del(&my_devices[i].cdev);
            kfree(my_devices[i].data);
        }
        kfree(my_devices);
    }

    // Unregister device
    unregister_chrdev_region(devno, 1);
}


int my_init_module(void)
{
    int result, error;
    dev_t dev = 0;

    // Allocate device
    if (my_major) {
        // Static allocation
        dev = MKDEV(my_major, my_minor);
        result = register_chrdev_region(dev, 1, "my_module");
    } else {
        // Dynamic allocation
        result = alloc_chrdev_region(&dev, my_minor, 1, "my_module");
        my_major = MAJOR(dev);
    }

    // Check if allocation was successful
    if (result < 0) {
        printk(KERN_WARNING "my_module: can't get major than %d" my_major);
        return result;
    }

    // Allocate memory for the devices
    my_devices->mem_size = my_mem_size;

    // Initialize the device
    my_devices->data = kmalloc(my_mem_size*sizeof(char), GFP_KERNEL);
    memset(my_devices->data, 0, my_mem_size*sizeof(char));

    // Initialize semaphore
    sema_init(&my_devices->sem, 1);

    // Initialize cdev
    cdev_init(&my_devices->cdev, &my_fops);
    my_devices->cdev.owner = THIS_MODULE;

    // Add device
    my_devices->cdev.ops = &my_fops;

    // Add device to the system
    error = cdev_add(&my_devices->cdev, dev, 1);

    // Check if adding was successful
    if (error)
        printk(KERN_NOTICE "Error %d adding my_module", error);
    else
        printk(KERN_INFO "my_module: module loaded with major number %d and minor number %d", my_major, my_minor);

    return 0;
}

module_init(my_init_module);
module_exit(my_cleanup_module);