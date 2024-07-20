#define pr_fmt(fmt) "%s(): line %d, " fmt, __func__, __LINE__

#include <linux/module.h>

static noinline void do_oops(void)
{
	*(int *)0x0 = 'a';
}

static int __init oops_test_init(void)
{
	pr_info("oops test init\n");
	do_oops();

	return 0;
}

static void __exit oops_test_exit(void)
{
	pr_info("oops test exit\n");
}

module_init(oops_test_init);
module_exit(oops_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jianyue Wu");
