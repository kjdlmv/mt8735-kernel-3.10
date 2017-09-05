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
#include <linux/wakelock.h>
#include "misc.h"

#include <mach/battery_common.h>

struct wake_lock m_lock;

//Motor
#define MOTOR_RST_PIN	(GPIO44 | 0x80000000)
#define MOTOR_BOOT0_PIN	(GPIO42 | 0x80000000)
#define MOTOR_POWEN_PIN	(GPIO43 | 0x80000000)
//Easy home
#define MOTOR_ZNJJPOW_PIN (GPIO2 | 0x80000000)
//Charger

#define CHG_EN_PIN (GPIO119 | 0x80000000)
#define CHG_CTL_PIN (GPIO120 | 0x80000000)

/*----------------------------------------------------------------------------*/

#define DEV_NAME   "misc-r150"
static struct cdev *misc_cdev;
static dev_t misc_dev;
static struct class *misc_class = NULL;
static struct device *misc_device = NULL;
 struct powerdata mcupowerdata;
 

 static int misc_release (struct inode *node, struct file *file)
 {
	 printk("misc_release !\n");
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

	return 0;
}
extern void mt_battery_update_status(void);
static uint16_t Voltage_FIFO[12]; 
uint16_t MoveAve_SMA( uint16_t NewData, uint16_t *MoveAve_FIFO, uint8_t SampleNum )
{
	uint8_t i = 0;
	uint16_t AveData = 0;
	uint32_t MoveAve_Sum = 0;
	
	uint16_t max,min;

	max = 0;
	min = 0xffff;
	
	for(i=0; i<SampleNum-1; i++)							//
	{
		MoveAve_FIFO[i] = MoveAve_FIFO[i+1];
		
		if(MoveAve_FIFO[i] == 0)
		{
				MoveAve_FIFO[i] = NewData;
		}
	}
	
	MoveAve_FIFO[SampleNum-1] = NewData;			// 
	
	for(i=0; i<SampleNum; i++)								//
	{
		MoveAve_Sum += MoveAve_FIFO[i];
		
		if(MoveAve_FIFO[i]<min)
		{
			min = MoveAve_FIFO[i];
		}
		
		if(MoveAve_FIFO[i]>max)
		{
			max = MoveAve_FIFO[i];
		}
	}
	
	MoveAve_Sum -= (min+max);
	
	AveData = (uint16_t)(MoveAve_Sum/(SampleNum-2));		//

	return AveData;
}

static long misc_unlocked_ioctl (struct file *pfile, unsigned int cmd, unsigned long param)
{
	void __user *argp = (void __user *)param;
	struct powerdata ppdata;
	unsigned int temp,tt;
	int err,ret,pptemp;
	unsigned int val;
	static bool firstflag=true;
	switch(cmd)
	{
		case POWER_DATA:
		if(copy_from_user(&ppdata, (struct powerdata*)param, sizeof(ppdata)))
		{
			err = -EFAULT;
			goto err_out;
		}
		if(ppdata.pdata >24000)
		{
			if(ppdata.pdata >28800)ppdata.pdata=28800;

			temp=(ppdata.pdata-24000)*1000;
		          tt=(28800-24000);
			val=temp/tt;

			pptemp=val/10;
		}
		else
		{
			pptemp=0;
		}		
		mcupowerdata.pdata = MoveAve_SMA(pptemp,Voltage_FIFO,12);
		
		if(firstflag)
		mt_battery_update_status();
		firstflag=false;
		
		printk("POWER_DATA==%d  percen=%d\n",ppdata.pdata,mcupowerdata.pdata);
	
		break;
	}
	

	return 1;
	
	err_out:
	return err;
}

static struct file_operations misc_fops = {
	.owner = THIS_MODULE,
	.open = misc_open,
	.write = misc_write,
	.read = misc_read,
	.release = misc_release,
	.unlocked_ioctl = misc_unlocked_ioctl,
};

static int __init misc_init(void)
{
	mcupowerdata.pdata=60;

	int ret = 0;

	memset(Voltage_FIFO,0,sizeof(Voltage_FIFO));
	
	printk("misc setting  init  finish--------\n");
	//Motor
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 1);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 0);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 1);
	//Easy home
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 1);	
	//Charger
	mt_set_gpio_dir(CHG_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(CHG_EN_PIN, 0);	
	mt_set_gpio_dir(CHG_CTL_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(CHG_CTL_PIN, 0);		
	wake_lock_init(&m_lock, WAKE_LOCK_SUSPEND, "misc"); //davie: forbid deep sleep for 5mic
	wake_lock(&m_lock);
	printk("misc setting  init  finish--------\n");

	
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
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 0);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 1);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 0);
	
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 0);	
	wake_lock_destroy(&m_lock);
}

module_init(misc_init);
module_exit(misc_exit);
MODULE_AUTHOR("Yongyida");
MODULE_DESCRIPTION("misc driver");
MODULE_LICENSE("GPL");

