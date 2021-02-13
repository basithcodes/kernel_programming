#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

dev_t device_number;
struct cdev test_cdev;
struct file_operations f_ops;

/* Create the init function */
static int __init pcd_module_init(void) {
	/* Create the device number */
	alloc_chrdev_region(&device_number, 0, 1, "test_device");

	/* Register the device with the VFS of the kernel*/
	cdev_init(&test_cdev, &f_ops);

	/* Create the class */

	return 0;
}

/* Create the exit function */
static void __exit pcd_module_cleanup(void) {

}

/* Load the module with kernel */
module_init(pcd_module_init);
module_exit(pcd_module_cleanup);

/* Module Info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abdul Basith");
MODULE_DESCRIPTION("A pseudo character device driver");


