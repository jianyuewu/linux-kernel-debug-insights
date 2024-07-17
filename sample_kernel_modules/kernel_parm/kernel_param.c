/*********************************************************************************
* A simple kernel module: hello
* *******************************************************************************/
#define pr_fmt(fmt) "%s(): line %d, " fmt, __func__, __LINE__

#include<linux/kernel.h>
#include<linux/delay.h>
#include<linux/init.h>
#include<linux/module.h>

static int test_rounds = 100;
module_param(test_rounds, int, 0644);

MODULE_PARM_DESC(test_rounds, "Max test rounds.");

static bool stop_test = false;
module_param(stop_test, bool, 0644);

MODULE_PARM_DESC(stop_test, "Write 1 or Y to stop test.");

static int __init kernel_param_test_init(void)
{
	int i;

	pr_info("Test start! Initial test rounds: %d\n", test_rounds);
	for (i = 0; i < test_rounds && !stop_test; i++) {
		pr_info("starting test round %d of total %d\n", i + 1,
		       test_rounds);
		msleep(1000);
	}
	test_rounds = i;
	return 0;
}

static void __exit kernel_param_test_exit(void)
{
	pr_info("stop_test in exit is %d.\n", stop_test);
	pr_info("test_rounds in exit is %d.\n", test_rounds);
}

module_init(kernel_param_test_init);
module_exit(kernel_param_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple module to use kernel parameter");
MODULE_AUTHOR("Jianyue Wu");
