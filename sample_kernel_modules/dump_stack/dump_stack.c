#define pr_fmt(fmt) "%s(): line %d, " fmt, __func__, __LINE__

#include <linux/module.h>

static int dump_stack_init(void)
{
	pr_info("dump_stack_init\n");
	dump_stack();

	return 0;
}

static void dump_stack_exit(void)
{
	pr_info("dump_stack exit\n");
}

module_init(dump_stack_init);
module_exit(dump_stack_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jianyue Wu");
