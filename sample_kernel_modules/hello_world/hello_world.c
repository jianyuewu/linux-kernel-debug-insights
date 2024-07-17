																			  /***************************************************************************//**
* A simple kernel module: hello
* *******************************************************************************/
#include<linux/module.h>

int add_int(int a, int b)
{
	return a + b;
}

EXPORT_SYMBOL_GPL(add_int);

static int __init hello_init(void)
{
	printk(KERN_INFO "Hello in init.\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "Hello in exit.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple hello world module");
MODULE_AUTHOR("Jianyue Wu");
