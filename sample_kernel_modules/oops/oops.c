#define pr_fmt(fmt) "%s(): line %d, " fmt, __func__, __LINE__

#include <linux/module.h>

static noinline void do_oops(void)
{
	*(int *)0x0 = 'a';
}

static int oops_init(void)
{
	pr_info("oops_init\n");
	do_oops();

	return 0;
}

static void oops_exit(void)
{
	pr_info("oops exit\n");
}

module_init(oops_init);
module_exit(oops_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jianyue Wu");
