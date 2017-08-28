#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include<linux/kthread.h>

#include <asm/uaccess.h>
#include <linux/hwmsen_dev.h>

#include <asm/uaccess.h>	/* copy_*_user */
#include <linux/semaphore.h>  
#include <linux/device.h>   /*class_create*/ 
#include <linux/slab.h>
#include <linux/delay.h>

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>

#include <mach/battery_common.h>
#include <misc.h>
#define DEV_NAME   "motor_updt_ctl"
static struct cdev *motor_cdev;
static dev_t m_dev;
static struct class *motor_class = NULL;
struct device *motor_device = NULL;
 
 static int motor_release (struct inode *node, struct file *file)
 {
	 printk("motor_release !\n");
	 return 0;

	 return 0;
 }
 static int motor_open (struct inode *inode, struct file *file)
 {
	 //MOTOR_INIT();
	 //POWER_EN();
	 printk("motor_open !\n");
	 return 0;
 }

static int motor_write(struct file *pfile, const char __user *from, size_t len, loff_t * offset)
{
	int reg_data = 0, ret = 0;
	char data[20] = {0};
	copy_from_user(data, from, len);
	printk("daviekuo %s %s", __func__, data);
	char dev = *(data+0);
	char sta = *(data+1);
	
	if (len != 0) {
		if(dev == 'A' || dev == 'a')
		{
			switch(sta)
			{
				case '1':
					
					break;
				case '0':
					
					break;
							
			}
		}
		else if(dev == 'B' || dev == 'b')
		{
			switch(sta)
			{
				case '1':
					mt_set_gpio_out(MOTOR_BOOT0_PIN, 1);
					mt_set_gpio_out(MOTOR_RST_PIN, 1);
					mdelay(10);					
					mt_set_gpio_out(MOTOR_POWEN_PIN, 0);
					mdelay(10);
					mt_set_gpio_out(MOTOR_POWEN_PIN, 1);
					mdelay(100);
					printk("daviekuo pull up STM32 to update !\n");
					break;
				case '0':
					mt_set_gpio_out(MOTOR_RST_PIN, 1);
					mt_set_gpio_out(MOTOR_BOOT0_PIN, 0);
					mdelay(10);
					mt_set_gpio_out(MOTOR_POWEN_PIN, 0);
					mdelay(10);
					mt_set_gpio_out(MOTOR_POWEN_PIN, 1);
					mdelay(10);
					printk("daviekuo pull down STM32 to out update !\n");
					break;
			}
		}
		else if(dev == 'C' || dev == 'c')
		{
			switch(sta)
			{
				case '1':
					
					break;
				case '0':
					
					break;
				default:
					ret = -1;
					break;
							
			}
		}
		else
			ret = -1;
	}
	else
		ret = -2;

	return ret;
}


static struct file_operations motor_fops = {
	.owner = THIS_MODULE,
	.open = motor_open,
	.write = motor_write,
	.release = motor_release,
};

/*----------------------------------------------------------------------------*/

static int __init motor_init(void)
{
	int err,ret;
	printk("daviekuo motor  init  finish--------\n");

	mt_set_gpio_mode(MOTOR_RST_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 1);

	mt_set_gpio_mode(MOTOR_BOOT0_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 0);
	
	mt_set_gpio_mode(MOTOR_POWEN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 1);

	
  	ret = alloc_chrdev_region(&m_dev, 0, 1, DEV_NAME);
         if (ret< 0) {
		 printk("motor  alloc_chrdev_region failed, %d", ret);
		return ret;
	}
	 motor_cdev= cdev_alloc();
	 if (motor_cdev == NULL) {
			 printk("motor cdev_alloc failed");
			 ret = -ENOMEM;
			 goto EXIT;
		 }
 	cdev_init(motor_cdev, &motor_fops);
	 motor_cdev->owner = THIS_MODULE;
	 ret = cdev_add(motor_cdev, m_dev, 1);
	 if (ret < 0) {
		  printk("Attatch file motor operation failed, %d", ret);
		 goto EXIT;
	 }
		 	 
	 motor_class = class_create(THIS_MODULE, DEV_NAME);
			 if (IS_ERR(motor_class)) {
				 printk("Failed to create class(motor)!\n");
				 return PTR_ERR(motor_class);
			 }
			 
	 motor_device = device_create(motor_class, NULL, m_dev, NULL,DEV_NAME);
	 if (IS_ERR(motor_device))
		 printk("Failed to create motor_dev device\n");
	

	printk("daviekuo motor  init  finish--------\n");

	return 0;

EXIT:
	if(motor_cdev != NULL)
    {
        cdev_del(motor_cdev);
        motor_cdev = NULL;
    }
	unregister_chrdev_region(m_dev, 1);
		 
	return ret;

}
/*----------------------------------------------------------------------------*/

static void __exit motor_exit(void)
{
	int err;

	if(motor_cdev != NULL)
	{
	    cdev_del(motor_cdev);
	    motor_cdev = NULL;
	}
    unregister_chrdev_region(m_dev, 1);

	return;

}

module_init(motor_init);
module_exit(motor_exit);
MODULE_AUTHOR("Yongyida");
MODULE_DESCRIPTION("motor driver");
MODULE_LICENSE("GPL");

