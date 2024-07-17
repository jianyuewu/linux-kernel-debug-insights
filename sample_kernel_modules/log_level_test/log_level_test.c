																												       /***************************************************************************//**
* A simple log level test module
* *******************************************************************************/

#define pr_fmt(fmt) "%s():line %d, " fmt, __func__, __LINE__

#include <linux/module.h>
#include <linux/device.h>

static struct device dummy_device = {
	.init_name = "dummy_device",
};

static int dev_print_with_return(void)
{
	dev_set_name(&dummy_device, "dummy_device");
	device_initialize(&dummy_device);

	dev_info(&dummy_device, "dev_() loglevel info.\n");
	return dev_err_probe(&dummy_device, -ENOMEM,
			     "Test return with not enough mem.\n");
}

static int __init log_level_test_init(void)
{
	int i;
	int ret;

	pr_emerg("pr_() loglevel emerg.\n");
	pr_alert("pr_() loglevel alert.\n");
	pr_crit("pr_() loglevel crit.\n");
	pr_err("pr_() loglevel err.\n");
	pr_warn("pr_() loglevel warn.\n");
	pr_notice("pr_() loglevel notice.\n");
	pr_info("pr_() loglevel info.\n");
	pr_debug("pr_() loglevel debug.\n");

	printk("printk() loglevel warn.\n");

	ret = dev_print_with_return();
	printk(KERN_INFO pr_fmt("dev_print_with_return() return value: %d\n"),
	       ret);
	for (i = 0; i < 100; i++) {
		pr_err_ratelimited("In print rate limiter, i:%d", i);
	}
	return 0;
}

static void __exit log_level_test_exit(void)
{
	pr_info("log_level_test exit.\n");
}

module_init(log_level_test_init);
module_exit(log_level_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A log level test module");
MODULE_AUTHOR("Jianyue Wu");
