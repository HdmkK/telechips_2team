#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");

#define LED1 117

static int __init led_init(void){
	printk("hello LED\n");
	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	gpio_set_value(LED1, 1);

	return 0;

}

static void __exit led_exit(void){
	printk("bye LED\n");
	gpio_set_value(LED1, 0);
	gpio_free(LED1);

}

module_init(led_init);
module_exit(led_exit);