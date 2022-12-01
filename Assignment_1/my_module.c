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
#include <linux/unistd.h>
#include <linux/sched.h>

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
    int my_mem_size;
};

struct my_dev my_device;

// ---------------------------file operations---------------------------

// Open
static int my_open(struct inode *inode, struct file *filp)
{
    struct my_dev *dev;

    // Get the device structure
    dev = container_of(inode->i_cdev, struct my_dev, cdev);

    // Save the device structure
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

    if (count > dev->my_mem_size)
        count = dev->my_mem_size;

    // Copy data from user space
    if (copy_from_user(dev->data, buf, count)) {
        retval = -EFAULT;
        goto out;
    }

    // print data
    printk(KERN_INFO, "My_write: %s", dev->data);

    // Update file position
    retval = count;

// Exit point for the function (release semaphore)
out:
    up(&dev->sem);
    return retval;
}

struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .write = my_write,
    .open = my_open,
    .release = my_release,
};

// ---------------------------proc file operations---------------------------
void my_cleanup_module(void)
{
    // Remove proc file entry from /proc
    dev_t devno = MKDEV(my_major, my_minor);

    // Remove device from system 
    cdev_del(&my_device.cdev);

    // Free memory
    kfree(my_device.data);

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
        result = register_chrdev_region(dev, 1, "my");
    } else {
        // Dynamic allocation
        result = alloc_chrdev_region(&dev, my_minor, 1, "my");
        my_major = MAJOR(dev);
    }

    // Check if allocation was successful
    if (result < 0) {
        printk(KERN_WARNING "my: can't get major than %d" my_major);
        return result;
    }

    // Allocate memory for the devices
    my_device.my_mem_size = my_mem_size;

    // Initialize the device
    my_device.data = kmalloc(my_mem_size*sizeof(char), GFP_KERNEL);
    memset(my_device.data, 0, my_mem_size*sizeof(char));

    // Initialize semaphore
    sema_init(&my_device.sem, 1);

    // Initialize cdev
    cdev_init(&my_device.cdev, &my_fops);
    my_device.cdev.owner = THIS_MODULE;

    // Add device
    my_device.cdev.ops = &my_fops;

    // Add device to the system
    error = cdev_add(&my_device.cdev, dev, 1);

    // Check if adding was successful
    if (error)
        printk(KERN_NOTICE "Error %d adding my_module", error);
    else
        printk(KERN_INFO "my_module: module loaded with major number %d and minor number %d", my_major, my_minor);

    return 0;
}

module_init(my_init_module);
module_exit(my_cleanup_module);