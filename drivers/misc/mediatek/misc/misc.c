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

#include <linux/wakelock.h>
#include <misc.h>
/*----------------------------------------------------------------------------*/
#define DEV_NAME   "misc_yyd"
static struct cdev *misc_cdev;
static dev_t misc_dev;
static struct class *misc_class = NULL;
struct device *misc_device = NULL;
extern  void aw2013_breath_all(int led0,int led1,int led2);
extern  int reset_mic_cx20810(void);
int danceflag=false;
static struct wake_lock m_lock;



kal_bool gsensor_data_switch = false;

#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_EARLYSUSPEND
  
  static void m_suspend( struct early_suspend *h )
  {
	//----------lifei--------------
		mt_set_gpio_out((GPIO68|0x80000000),0);
		mt_set_gpio_out((GPIO2|0x80000000),0);
		mt_set_gpio_out((GPIO43|0x80000000),0);
		mt_set_gpio_out((GPIO121|0x80000000),0);
	//----------------------------	

  }
  
  static void m_resume( struct early_suspend *h )
  {
 	
	//----------lifei--------------
		mt_set_gpio_out((GPIO68|0x80000000),1);
		mt_set_gpio_out((GPIO2|0x80000000),1);
		mt_set_gpio_out((GPIO43|0x80000000),1);
		mt_set_gpio_out((GPIO121|0x80000000),1);
	//----------------------------	
	
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

	
 }
 static int misc_open (struct inode *inode, struct file *file)
 {
	 return nonseekable_open(inode, file);

 }

static int misc_write(struct file *pfile, const char __user *buf, size_t len, loff_t * offset)
{
	 char pbuf[50] ,**argv=pbuf;
	
	   if(copy_from_user(pbuf, buf,len))
	   {
		   return	 -EFAULT;  
	   }
	    printk("111111%c,%d\n",pbuf[0],pbuf[1]);
	  if(pbuf[0] == 'A')
	  {
		if(pbuf[1] == '0'){danceflag=false;aw2013_breath_all(0,0,0);}
		else if(pbuf[1] == '1'){danceflag=true;aw2013_breath_all(1,1,1);}	
	  }
	  else if(pbuf[0] == 'B') 
	    {
	          if(pbuf[1] == 'A')
	          return reset_mic_cx20810();
	    }
		else if(pbuf[0] == 'G') 
	    {
	        if(pbuf[1] == '0')
				gsensor_data_switch = KAL_FALSE;
			else if(pbuf[1] == '1')
				gsensor_data_switch = KAL_TRUE;
	    }
		
	return len;
}
static int misc_read(struct file *pfile, char __user *to, size_t len, loff_t *offset)
{
	printk("misc_read !\n");
	
	
	return 0;
}


static long misc_unlocked_ioctl (struct file *pfile, unsigned int cmd, unsigned long param)
{
			
	 return 0;
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
	int ret = 0;
	printk("misc setting  init  finish--------\n");

	#ifdef CONFIG_HAS_EARLYSUSPEND
	#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&misc_early_suspend_handler);
	#endif
	#endif
	
	//Motor
	mt_set_gpio_mode(MOTOR_RST_PIN,GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 1);
	
	mt_set_gpio_mode(MOTOR_BOOT0_PIN,GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 0);
	
	mt_set_gpio_mode(MOTOR_POWEN_PIN,GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 1);
	//Easy home
	mt_set_gpio_mode(MOTOR_ZNJJPOW_PIN,GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 1);	
	//Charger
	mt_set_gpio_mode(CHG_EN_PIN,GPIO_MODE_00);
	mt_set_gpio_dir(CHG_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(CHG_EN_PIN, 0);	
	
	mt_set_gpio_mode(CHG_CTL_PIN,GPIO_MODE_00);
	mt_set_gpio_dir(CHG_CTL_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(CHG_CTL_PIN, 0);		
	
//	mt_set_gpio_mode(CHG_USB_EN,GPIO_DIR_OUT);
//	mt_set_gpio_dir(CHG_USB_EN, GPIO_DIR_OUT);
//	mt_set_gpio_out(CHG_USB_EN, 1);

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
	mt_set_gpio_mode(MOTOR_RST_PIN,GPIO_DIR_OUT);
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 0);
	
	mt_set_gpio_mode(MOTOR_BOOT0_PIN,GPIO_DIR_OUT);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 1);
	
	mt_set_gpio_mode(MOTOR_POWEN_PIN,GPIO_DIR_OUT);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 0);
	
	mt_set_gpio_mode(MOTOR_ZNJJPOW_PIN,GPIO_DIR_OUT);
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 0);	
	
	wake_lock_destroy(&m_lock);
//	mt_set_gpio_mode(CHG_USB_EN,GPIO_DIR_OUT);
//	mt_set_gpio_dir(CHG_USB_EN, GPIO_DIR_OUT);
//	mt_set_gpio_out(CHG_USB_EN, 0);
	
}

module_init(misc_init);
module_exit(misc_exit);
MODULE_AUTHOR("Yongyida");
MODULE_DESCRIPTION("misc driver");
MODULE_LICENSE("GPL");

