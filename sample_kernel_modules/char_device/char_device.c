#define pr_fmt(fmt) "%s(): line %d, " fmt, __func__, __LINE__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>

#define DEVICE_NAME "mychardev"
#define TEST_CDEV_BUF_SIZE (1024 * 1024)	// 1 MB buffer

static dev_t dev_number;
static struct cdev *my_cdev;
static char *test_cdev_buf;
static size_t test_cdev_buf_size = 0;

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t,
			    loff_t *);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static int __init mychardev_init(void)
{
	int result;

	// Allocate device number
	result = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
	if (result < 0) {
		pr_err("Failed to allocate device number\n");
		return result;
	}

	// Allocate char device structure
	my_cdev = cdev_alloc();
	if (!my_cdev) {
		unregister_chrdev_region(dev_number, 1);
		pr_err("Failed to allocate cdev structure\n");
		return -ENOMEM;
	}

	// Initialize char device
	cdev_init(my_cdev, &fops);
	my_cdev->owner = THIS_MODULE;

	// Add char device to the system
	result = cdev_add(my_cdev, dev_number, 1);
	if (result < 0) {
		kfree(my_cdev);
		unregister_chrdev_region(dev_number, 1);
		pr_err("Failed to add cdev to system\n");
		return result;
	}

	test_cdev_buf = kmalloc(TEST_CDEV_BUF_SIZE, GFP_KERNEL);
	if (!test_cdev_buf) {
		cdev_del(my_cdev);
		unregister_chrdev_region(dev_number, 1);
		pr_err("Failed to allocate memory for buffer\n");
		return -ENOMEM;
	}

	pr_info("Device registered with major number %d\n", MAJOR(dev_number));
	pr_info("Can try read and write with /dev/%s\n", DEVICE_NAME);
	return 0;
}

static void __exit mychardev_exit(void)
{
	kfree(test_cdev_buf);
	cdev_del(my_cdev);
	unregister_chrdev_region(dev_number, 1);
	pr_info("Goodbye!\n");
}

/* Called when a process tries to open the device file */
static int device_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	pr_info("Device opened\n");
	return 0;
}

/* Called when a process closes the device file */
static int device_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	pr_info("Device closed\n");
	return 0;
}

/* Called when a process reads from the device file */
static ssize_t device_read(struct file *filp, char __user * buffer,
			   size_t length, loff_t * offset)
{
	size_t bytes_to_read;
	if (*offset >= test_cdev_buf_size) {
		return 0;
	}
	bytes_to_read = min(length, (size_t)(test_cdev_buf_size - *offset));
	if (copy_to_user(buffer, test_cdev_buf + *offset, bytes_to_read)) {
		return -EFAULT;
	}
	*offset += bytes_to_read;
	pr_info("Read %zu bytes from device\n", bytes_to_read);
	return bytes_to_read;
}

/* Called when a process writes to the device file */
static ssize_t device_write(struct file *filp, const char __user * buffer,
			    size_t length, loff_t * offset)
{
	size_t bytes_to_write;
	if (*offset >= TEST_CDEV_BUF_SIZE) {
		return -ENOSPC;
	}
	bytes_to_write = min(length, (size_t)(TEST_CDEV_BUF_SIZE - *offset));
	if (copy_from_user(test_cdev_buf + *offset, buffer, bytes_to_write)) {
		return -EFAULT;
	}
	*offset += bytes_to_write;
	test_cdev_buf_size = max(test_cdev_buf_size, (size_t)(*offset));
	pr_info("Wrote %zu bytes to device\n", bytes_to_write);
	return bytes_to_write;
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jianyue Wu");
MODULE_DESCRIPTION("A simple character device driver with a 1MB buffer");
