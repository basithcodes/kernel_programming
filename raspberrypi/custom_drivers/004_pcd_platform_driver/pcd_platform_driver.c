#include <linux/fs.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include "platform.h"

int check_permission(int device_permission, int device_access_mode) {
	if (device_permission == RDWR) 
		return 0;
	if (device_permission == RDONLY && (device_access_mode & FMODE_READ) && !(device_access_mode & FMODE_WRITE))
		return 0;
	if (device_permission == WRONLY && (device_access_mode & FMODE_WRITE) && !(device_access_mode & FMODE_READ))
		return 0;
	return -EPERM;
}


int pcd_open(struct inode *inode, struct file *filp) {
	return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) {
	return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) {
	return -ENOMEM;
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) {
	return 0;
}

int pcd_release(struct inode *inode, struct file *filp) {
	return 0;
}

struct file_operations pcd_fops = {
	.open = pcd_open,
	.read = pcd_read,
	.write = pcd_write,
	.llseek = pcd_lseek,
	.release = pcd_release,
	.owner = THIS_MODULE
};

static int __init pcdev_platform_driver_init(void) {
	return 0;
}

static void __exit pcdev_platform_driver_exit(void) {

}

module_init(pcdev_platform_driver_init);
module_exit(pcdev_platform_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Driver for pseudo platform device");
MODULE_AUTHOR("Abdul Basith");
