#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO, KERN_ALERT */
#include <linux/init.h> /* Needed for __init and __exit macros */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hoelzel");
MODULE_DESCRIPTION("A simple Hello World module");

static int __init hello_init(void)
{
	printk(KERN_INFO "Hello world!\n");

	/* A non 0 return means init_module failed; module can't be loaded */
	return 0;
}

static void __exit hello_cleanup(void)
{
	printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
