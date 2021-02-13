#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

/* Create the init function */
static int __init pcd_module_init(void) {

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


