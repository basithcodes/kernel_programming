#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

/* Create the fops function definitions */
int test_open();

dev_t device_number;
struct cdev test_cdev;
struct file_operations f_ops;
struct class *class_test;
struct device *device_test;

/* Create the init function */
static int __init pcd_module_init(void) {
	/* Create the device number */
	alloc_chrdev_region(&device_number, 0, 1, "test_device_number");
	pr_info("Device Number <major>:<minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

	/* Initialize the cdev with fops */
	cdev_init(&test_cdev, &f_ops);

	/* Register the device with the VFS of the kernel*/
	cdev_add(&test_cdev, device_number, 1);

	/* Create the device class under sys/class */
	class_test = class_create(THIS_MODULE, "test_class");

	/* Create the device file under sys/class 
	 * Or populate the device info in sys/class */
	device_test = device_create(class_test, NULL, device_number, NULL, "test_device");

	pr_info("Module init was successfull\n");

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
