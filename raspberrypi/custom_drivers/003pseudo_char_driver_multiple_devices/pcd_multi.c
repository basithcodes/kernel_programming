#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/module.h>

#define MEM_SIZE_MAX_PCDEV1 1024
#define MEM_SIZE_MAX_PCDEV2 512
#define MEM_SIZE_MAX_PCDEV3 1024
#define MEM_SIZE_MAX_PCDEV4 512

#define RDONLY 0x01
#define WRONLY 0x02
#define RDWR 0x03

#define NO_OF_DEVICES 4

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt, __func__

/* pseudo device's memory */
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];

/* Device private data structure */
struct pcdev_private_data {
	char *buffer;
	unsigned size;
	const char *serial_number;
	int perm;
	struct cdev cdev;
};

/* Driver private data structure */
struct pcdrv_private_data {
	int total_devices;
	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
	/* this holds the device number */
	dev_t device_number;
	struct class *class_pcd;
	struct device *device_pcd;
};

struct pcdrv_private_data pcdrv_data = {
	.total_devices = NO_OF_DEVICES,
	.pcdev_data = {
		[0] = {
			.buffer = device_buffer_pcdev1,
			.size = MEM_SIZE_MAX_PCDEV1,
			.serial_number = "PCDEV1",
			.perm = RDONLY
		},

		[1] = {
			.buffer = device_buffer_pcdev2,
			.size = MEM_SIZE_MAX_PCDEV2,
			.serial_number = "PCDEV2",
			.perm = WRONLY
		},

		[2] = {
			.buffer = device_buffer_pcdev3,
			.size = MEM_SIZE_MAX_PCDEV3,
			.serial_number = "PCDEV3",
			.perm = RDWR
		},

		[3] = {
			.buffer = device_buffer_pcdev4,
			.size = MEM_SIZE_MAX_PCDEV4,
			.serial_number = "PCDEV4",
			.perm = RDWR
		}
	}

};

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) {

	loff_t temp;

	int max_size;

	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

	max_size = pcdev_data->size;

	pr_info("lseek requested \n");
	pr_info("Current file position = %lld\n", filp->f_pos);

	switch (whence) {
		case SEEK_SET:
			if ((offset > max_size) || (offset < 0)) 
				return -EINVAL;
			filp->f_pos = offset;
			break;

		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if ((temp > max_size) || temp < 0) 
				return -EINVAL;
			filp->f_pos = temp;
			break;

		case SEEK_END:
			temp = max_size + offset;
			if ((temp > max_size) || temp < 0)
				return -EINVAL;
			filp->f_pos = temp;
			break;

		default:
			return -EINVAL;
	}

	pr_info("New value of file position = %lld\n", filp->f_pos);

	return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count,	loff_t *f_pos) {

	int max_size;
	/* Get the private data of the device */
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

	pr_info("read requested for %zu bytes \n", count);
	pr_info("Current file position = %lld\n", *f_pos);

	/* save buffer size in a variable */
	max_size = pcdev_data->size;

	/* Adjust the count */
	if ((*f_pos + count) > max_size)
		count = max_size - *f_pos;

	/* Copy to User */
	if (copy_to_user(buff, pcdev_data->buffer + (*f_pos), count))
		return -EFAULT;

	/* Update the current file position */
	*f_pos += count;

	pr_info("Number of bytes successfully read = %zu\n", count);
	pr_info("Updated file position = %lld\n", *f_pos);

	/* Return number of bytes which have been successfully read */
	return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) {

	int max_size;

	/* Get the private data of the device */
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

	pr_info("Write requested for %zu bytes \n", count);
	pr_info("Current file position = %lld\n", *f_pos);

	max_size = pcdev_data->size;

	/* Adjust the count */
	if ((*f_pos + count) > max_size)
		count = max_size - *f_pos;

	if(!count) {
		pr_err("No space left on the device");
		return -ENOMEM;
	}

	/* Copy from user */
	if (copy_from_user(pcdev_data->buffer + (*f_pos), buff, count))
		return -EFAULT;

	/* Update the current file position */
	*f_pos += count;

	pr_info("Number of bytes successfully written = %zu\n", count);
	pr_info("Updated file position = %lld\n", *f_pos);

	/* Return number of bytes written to memory */
	return count;
}

int check_permission(int device_permission, unsigned int access_mode) {

	if (device_permission == RDWR)
		return 0;

	/* This ensures that device is in read only mode */
	if ((device_permission == RDONLY) && !(access_mode & FMODE_WRITE))
		return 0;

	/* This ensures that device is in write only mode */
	if ((device_permission == WRONLY) && !(access_mode & FMODE_READ))
		return 0;

	return -EPERM;
}

int pcd_open(struct inode *inode, struct file *filp) {
	int ret;

	int minor_number;

	struct pcdev_private_data *pcdev_data;

	/* Identify which device file has been invoked and print the information of it */
	minor_number = MINOR(inode->i_rdev);
	pr_info("%d device file has been requested\n", minor_number);
	
	/* Get that device file information  */
	pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);

	/* Send the device file information to other methods */
	filp->private_data = pcdev_data;

	/* Check the permission of that device */
	ret = check_permission(pcdev_data->perm, filp->f_mode);
	(!ret)?pr_info("Permission granted\n"):(pr_info("Permission denied\n"));

	return ret;
}

int pcd_release(struct inode *inode, struct file *filp) {
	pr_info("Release was successfull\n");
	return 0;
}

/* file operations of the driver */
struct file_operations pcd_fops = {
	.open = pcd_open,
	.write = pcd_write,
	.read = pcd_read,
	.llseek = pcd_lseek,
	.release = pcd_release,
	.owner = THIS_MODULE
};

static int __init pcd_driver_init(void) {

	int ret;
	int i = 0;

	/* Dynamically allocate a device number */
	ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, "pcd_devices");
	if (ret < 0) {
		pr_err("Allocating device number for character device failed\n");
		goto out;
	}

	/* Create device class under /sys/class */
	pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
	if (IS_ERR(pcdrv_data.class_pcd)) {
		pr_err("Class creation failed\n");
		ret = PTR_ERR(pcdrv_data.class_pcd);
		goto unreg_chrdev;
	}

	for (i = 0; i < NO_OF_DEVICES; i++) {
		pr_info("Device Number <major>:<minor> = %d:%d\n",
				MAJOR(pcdrv_data.device_number+i), 
				MINOR(pcdrv_data.device_number+i));

		/* Initialize the cdev structure with fops */
		cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);

		/* Register a device (cdev structure) with VFS */
		pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
		ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number + i, 1);
		if (ret < 0) {
			pr_err("cdev register failed\n");
			goto cdev_del;
		}

		/* Populate the sysfs with device information */
		pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, 
				pcdrv_data.device_number + i, NULL, "pcdev-%d", i+1);
		if (IS_ERR(pcdrv_data.device_pcd)) {
			pr_err("Device creation failed\n");
			ret = PTR_ERR(pcdrv_data.device_pcd);
			goto class_destroy;
		}

	}
	pr_info("Module init was successfull\n");
	return 0;

cdev_del:
class_destroy:
	for (; i >= 0; i--) {
		device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
	class_destroy(pcdrv_data.class_pcd);

unreg_chrdev:
	unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);

out:
	pr_info("Module Insertion Failed\n");
	return ret;
}

static void __exit pcd_driver_cleanup(void) {
	int i;

	for (i = 0; i < NO_OF_DEVICES; i++) {
		device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
	class_destroy(pcdrv_data.class_pcd);

	for (i = 0; i < NO_OF_DEVICES; i++) {
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
	unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);
	pr_info("Module Unloaded\n");

}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abdul Basith");
MODULE_DESCRIPTION("A pseudo char driver for multiple devices");
