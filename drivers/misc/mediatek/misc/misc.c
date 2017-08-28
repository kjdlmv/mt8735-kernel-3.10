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
#include <linux/earlysuspend.h>

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
#include <mach/mt_boot_common.h>

#include <linux/wakelock.h>
#include <misc.h>

#define DEV_NAME   "misc-yyd"
static struct cdev *misc_cdev;
static dev_t misc_dev;
static struct class *misc_class = NULL;
struct device *misc_device = NULL;
 struct wake_lock yyd_m_lock;


#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_EARLYSUSPEND
 
 static void m_suspend( struct early_suspend *h )
 {
#ifdef CONFIG_MISC_Y20A

#else


#endif

 //----------------li fei-----------------
	 mt_set_gpio_out((GPIO101|0x80000000),0);
 //-----------------------------------

 }
 
 static void m_resume( struct early_suspend *h )
 {
 #ifdef CONFIG_MISC_Y20A

#else


#endif

 //----------------li fei-----------------
	 mt_set_gpio_out((GPIO101|0x80000000),1);
 //-----------------------------------

 }
 
 static struct early_suspend misc_early_suspend_handler = {
	 .level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 1,
	 .suspend = m_suspend,
	 .resume = m_resume,
 };
 
#endif
#endif


 static int misc_release (struct inode *node, struct file *file)
 {
	 printk("misc_release !\n");
	 return 0;

	 return 0;
 }
 static int misc_open (struct inode *inode, struct file *file)
 {
	 printk("misc_open !\n");
	 return 0;
 }

static int misc_write(struct file *pfile, const char __user *from, size_t len, loff_t * offset)
{
	printk("misc_write !\n");
	return 0;
}
static int misc_read(struct file *pfile, char __user *to, size_t len, loff_t *offset)
{
	printk("misc_read !\n");
	
	char data[10] = {0};
	
	if(mt_get_gpio_in(CHG_DET_PIN))
	{
		data[0] = '1';
	}
	else
		data[0] = '0';
	data[1] = '\n';
	copy_to_user(to, data, 2);
	
	return 2;
}


static struct file_operations misc_fops = {
	.owner = THIS_MODULE,
	.open = misc_open,
	.write = misc_write,
	.read = misc_read,
	.release = misc_release,
};

static int __init misc_init(void)
{
	int ret = 0;

	BOOTMODE bootmode = NORMAL_BOOT;
	printk("misc setting  init  finish--------\n");

	#ifdef CONFIG_HAS_EARLYSUSPEND
	#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&misc_early_suspend_handler);
	#endif
	#endif

	
//Motor
	mt_set_gpio_mode(MOTOR_RST_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 1);

	mt_set_gpio_mode(MOTOR_BOOT0_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	//mt_set_gpio_pull_enable(MOTOR_BOOT0_PIN, FALSE);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 0);

	mt_set_gpio_mode(MOTOR_POWEN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 1);

	mt_set_gpio_mode(CHG_DET_EN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(CHG_DET_EN_PIN, GPIO_DIR_OUT);
	
	mt_set_gpio_dir(CHG_DET_PIN, GPIO_DIR_IN);
	
	mt_set_gpio_pull_enable(CHG_DET_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(CHG_DET_PIN, GPIO_PULL_UP);	
/*
//Easy home
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 1);			
*/
//Ircam
	mt_set_gpio_mode(IRCAM_PWREN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(IRCAM_PWREN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(IRCAM_PWREN_PIN, 1);
	
	wake_lock_init(&yyd_m_lock, WAKE_LOCK_SUSPEND, "misc"); //davie: forbid deep sleep for 5mic
	wake_lock(&yyd_m_lock);

//Charger DETECT
	bootmode = get_boot_mode();
	
	if(KERNEL_POWER_OFF_CHARGING_BOOT == bootmode || LOW_POWER_OFF_CHARGING_BOOT == bootmode)
	{
		mt_set_gpio_out(CHG_DET_EN_PIN, 0);
		printk("misc setting  CHG_DET_EN_PIN, 0\n");
	}
	else
	{
		mt_set_gpio_out(CHG_DET_EN_PIN, 1);
		printk("misc setting  CHG_DET_EN_PIN,1\n");
	}
	

	ret = alloc_chrdev_region(&misc_dev, 0, 1, DEV_NAME);
		 if (ret< 0) {
		 printk("misc alloc_chrdev_region failed, %d", ret);
		return ret;
	}
	 misc_cdev= cdev_alloc();
	 if (misc_cdev == NULL) {
			 printk("misc cdev_alloc failed");
			 ret = -ENOMEM;
			 goto EXIT;
		 }
	cdev_init(misc_cdev, &misc_fops);
	 misc_cdev->owner = THIS_MODULE;
	 ret = cdev_add(misc_cdev, misc_dev, 1);
	 if (ret < 0) {
		  printk("Attatch file misc operation failed, %d", ret);
		 goto EXIT;
	 }
			 
	 misc_class = class_create(THIS_MODULE, DEV_NAME);
			 if (IS_ERR(misc_class)) {
				 printk("Failed to create class(misc)!\n");
				 return PTR_ERR(misc_class);
			 }
			 
	 misc_device = device_create(misc_class, NULL, misc_dev, NULL,DEV_NAME);
	 if (IS_ERR(misc_device))
		 printk("Failed to create misc_dev device\n");
	

	printk("daviekuo misc	init  finish--------\n");

	return 0;
	
EXIT:
	if(misc_cdev != NULL)
	{
		cdev_del(misc_cdev);
		misc_cdev = NULL;
	}
	unregister_chrdev_region(misc_dev, 1);

	printk("misc device  init  failed--------\n");			 

	return ret;
}
/*----------------------------------------------------------------------------*/

static void __exit misc_exit(void)
{
	//Motor
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 0);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 1);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 0);
/*	
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 0);	
*/
	mt_set_gpio_dir(IRCAM_PWREN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(IRCAM_PWREN_PIN, 0);
	
	wake_lock_destroy(&yyd_m_lock);
}

module_init(misc_init);
module_exit(misc_exit);
MODULE_AUTHOR("Yongyida");
MODULE_DESCRIPTION("misc driver");
MODULE_LICENSE("GPL");

