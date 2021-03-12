#include <linux/device/class.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/device.h>

#define NO_OF_DEVICES 4

#define DEVICE_ONE_MEM_SIZE 512
#define DEVICE_TWO_MEM_SIZE 1024
#define DEVICE_THREE_MEM_SIZE 512
#define DEVICE_FOUR_MEM_SIZE 1024

#define RDONLY 0x01
#define WRONLY 0x02
#define RDWR 0x03

/* Device Memory */
char device_one_buffer[DEVICE_ONE_MEM_SIZE];
char device_two_buffer[DEVICE_TWO_MEM_SIZE];
char device_three_buffer[DEVICE_THREE_MEM_SIZE];
char device_four_buffer[DEVICE_FOUR_MEM_SIZE];

/* Create the device private data structure */
struct device_private_data {
	char *buffer;
	char *serial_number;
	unsigned size;
	int permission;
	struct cdev device_cdev;
};

/* Create the driver private data structure */
struct driver_private_data {
	int number_of_devices;
	struct device_private_data device_data[NO_OF_DEVICES];
	dev_t device_number;
	struct class *multi_class;
	struct device *multi_device;
};

struct driver_private_data driver_data = {
	.number_of_devices = 4,
	.device_data = {

		[0] = {
			.buffer = device_one_buffer,
			.serial_number = "pcdev1",
			.size = DEVICE_ONE_MEM_SIZE,
			.permission = RDONLY,
		},

		[1] = {
			.buffer = device_two_buffer,
			.serial_number = "pcdev2",
			.size = DEVICE_TWO_MEM_SIZE,
			.permission = WRONLY
		},

		[2] = {
			.buffer = device_three_buffer,
			.serial_number = "pcdev3",
			.size = DEVICE_THREE_MEM_SIZE,
			.permission = RDWR
		},

		[3] = {
			.buffer = device_four_buffer,
			.serial_number = "pcdev4",
			.size = DEVICE_FOUR_MEM_SIZE,
			.permission = RDWR 
		}
	},
};
int check_permission(void) {
	return 0;
}

/* Create the open method */
int multi_open(struct inode *inode, struct file *filp) {

	int ret;
	int minor_number;

	/* Inode contains the information of specific device file and it contains device number as well */
	minor_number = MINOR(inode->i_rdev); // i_rdev is device number of specific device
	
	pr_info("Device file %d is opened", minor_number);

	/* Driver open method should supply private data to other methods, 
	 * struct file contains member element *private_data,
	 * store the device specific data in private data pointer */
	struct device_private_data *device_data;

	/* container of macro extracts the address of device_private_data structure using member element cdev*/
	device_data = container_of(inode->i_cdev, struct device_private_data, device_cdev);

	/* Supply the address of device_private_data to struct file member element(private data) */
	filp->private_data = device_data;

	/* Check the device file permission whether its read only write only etc */
	ret = check_permission(); // returns zero on success

	(!ret)?(pr_info("Open was successfull\n")):(pr_info("Open was unsuccessfull\n"));

	return ret;
}

/* Create the read method */
ssize_t multi_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) {

	return count;
}

/* Create the write method */
ssize_t multi_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) {
	return -ENOMEM;
}

/* Create the llseek method */
loff_t multi_llseek(struct file *filp, loff_t f_pos, int count) {
	return count;
}

/* Create the release method */
int multi_release(struct inode *inode, struct file *filp) {
	return 0;
}

/* Create file operations structure */
struct file_operations multi_fops = {
	.open = multi_open,
	.read = multi_read,
	.write = multi_write,
	.llseek = multi_llseek,
	.release = multi_release,
	.owner = THIS_MODULE
};

/* Create the init function */
static int __init multiple_device_init(void) {
	int ret;
	int i;

	/* Register the device number with the VFS  */
	ret = alloc_chrdev_region(&driver_data.device_number, 0, NO_OF_DEVICES, "multi-device");
	if (ret < 0) {
		pr_info("Device registration failed\n");
		goto out;
	}

	/* Create the class in /sys/class */
	driver_data.multi_class = class_create(THIS_MODULE, "multi_device_class");
	if (IS_ERR(driver_data.multi_class)) {
		pr_err("Class creation failed\n");
		ret = PTR_ERR(driver_data.multi_class);
		goto unreg_chrdev;
	}

	for (i = 0; i < NO_OF_DEVICES; i++) {
		/* Print the device number */
		pr_info(" <Major>:<Minor> = %d:%d \n", MAJOR(driver_data.device_number+i), MINOR(driver_data.device_number+i));

		/* Initialize the cdev with file operations */
		cdev_init(&driver_data.device_data[i].device_cdev, &multi_fops);

		/* Add the cdev to the VFS */
		driver_data.device_data[i].device_cdev.owner = THIS_MODULE;
		ret = cdev_add(&driver_data.device_data[i].device_cdev, driver_data.device_number+i, NO_OF_DEVICES);
		if (ret < 0) {
			pr_err("Cdev registration failed\n");
			goto cdev_delete;
		}

		/* Create the devices under class */
		driver_data.multi_device = device_create(driver_data.multi_class, NULL, driver_data.device_number+i, NULL, "multi_device: %d", i);
		if (IS_ERR(driver_data.multi_device)) {
			pr_err("Device creation failed\n");
			ret = PTR_ERR(driver_data.multi_device);
			goto class_device_destroy;
		}


	}

cdev_delete:
class_device_destroy:
	for (i = NO_OF_DEVICES-1; i >= 0; i--) {
		device_destroy(driver_data.multi_class, driver_data.device_number+i);
		cdev_del(&driver_data.device_data[i].device_cdev);
	}
	class_destroy(driver_data.multi_class);

unreg_chrdev:
	unregister_chrdev_region(driver_data.device_number, NO_OF_DEVICES);

out:
	return ret;

}

/* Create the exit function */
static void __exit multiple_device_exit(void) {
	int i;

	for (i = 0; i < NO_OF_DEVICES; i++) {
		device_destroy(driver_data.multi_class, driver_data.device_number+i);
		cdev_del(&driver_data.device_data[i].device_cdev);
	}
	class_destroy(driver_data.multi_class);

	unregister_chrdev_region(driver_data.device_number, NO_OF_DEVICES);
	pr_info("Module Unloaded Successfully\n");
}

/* Initialize the init and exit functions */
module_init(multiple_device_init);
module_exit(multiple_device_exit);

/* Write the module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abdul Basith");
MODULE_DESCRIPTION("A driver to handle multiple devices");
