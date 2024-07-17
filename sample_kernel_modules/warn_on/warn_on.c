#define pr_fmt(fmt) "%s(): line %d, " fmt, __func__, __LINE__

#include <linux/module.h>

static int warn_on_once_init(void)
{
	pr_info("warn_on_once init\n");
	WARN_ON_ONCE(1);
	// WARN_ON(1);
	// BUG_ON(1);

	return 0;
}

static void warn_on_once_exit(void)
{
	pr_info("warn_on_once exit\n");
}

module_init(warn_on_once_init);
module_exit(warn_on_once_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jianyue Wu");
