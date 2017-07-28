
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/device.h>   /*class_create*/ 
#include <asm/uaccess.h>	/* copy_*_user */
#include <linux/input.h>
#include <linux/string.h>

#include "tpd.h"
extern struct tpd_device *tpd;

#define DEV_NAME   "ircam"
static struct cdev *ircam_cdev;
static dev_t ic_dev;
static struct class *ircam_class = NULL;
struct device *ircam_device = NULL;


static void tpd_down(int x, int y, int d) {
	 input_report_key(tpd->dev, BTN_TOUCH, 1);
	 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 20);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	 input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, d); 
	 input_mt_sync(tpd->dev);
}
  
static void tpd_up(int x, int y) {
	input_report_key(tpd->dev, BTN_TOUCH, 0);	  
}

static int ir_cam_report(int x, int y, int d)
{ 
	 int i=0;
	 
	 if(d > 0) 
	 {
		 //for(i =0; i<point_num; i++)
		 {
			//tpd_down(720-cinfo.x[i], 1280-cinfo.y[i], cinfo.id[i]);
			//tpd_down(1280-cinfo.y[i], cinfo.x[i], cinfo.id[i]);
			//tpd_down(x, 854-y, 0);
			tpd_down((1280- y) * 720 / 1280, (720 - x) * 1280 / 720, 0);	// Modified by Bright
			//tpd_down((854- y) * 480 / 854, (480 - x) * 854 / 480, 0);	// Modified by Bright
		 }
		 input_sync(tpd->dev);
	 }
	 else  
	 {
		   //tpd_up(x, 854-y);
		   //tpd_up(720-cinfo.x[0], 1280-cinfo.y[0]);
		  tpd_up((1280 - y) * 720 / 1280, (720 - x) * 1280 / 720); // Modified by Bright		   
		  //tpd_up((854 - y) * 480 / 854, (480 - x) * 854 / 480); // Modified by Bright		
		  input_sync(tpd->dev);
	 }
	 printk(" x = %d, y = %d, d = %d\n", x, y, d);

	return 0;
}

static long ircam_unlocked_ioctl(struct file *pfile, unsigned int cmd, unsigned long param)
{
	 return 0;	
}


static int ircam_release (struct inode *node, struct file *file)
{
 	return 0;
}

static int ircam_open (struct inode *inode, struct file *file)
{
	printk(KERN_INFO"/dev/ircam open success!\n");
	return 0;
}

static int ircam_write(struct file *pfile, const char __user *from, size_t len, loff_t * offset)
{
	int i = 0, j = 0, k = 0, len_t = 0;
	int touch[3] = {0}; 

	char data[128] = {0};
	char tmp[8] = {0};
	copy_from_user(data, from, len);
	data[len] = '\0';
	printk("daviekuo %s received data: %s",__FUNCTION__, data);
	len_t = strlen(data);
/*
	if(len_t < len)	
	{
		printk(KERN_ERR" %s len error !\n", __FUNCTION__);
		return -1;
	}
	*/
	for(i = 0, j = 0; i < 3 && j<len_t; i++)
	{
		for(k = 0; data[j+k] != '#';k++)
			;
		memcpy(tmp, &data[j], k);
		j=j+k+1;
		tmp[k] = '\0';
		touch[i] = simple_strtoul(tmp, 0, 10);
	}
	
	ir_cam_report(touch[0],touch[1],touch[2]);
	printk("daviekuo, x = %d y = %d  d = %d \n",touch[0],touch[1],touch[2]);
	printk(KERN_ERR" %s len = %d len_t = %d !\n", __FUNCTION__, len,len_t);

	return 0;
}

static int ircam_read (struct file *pfile, char __user *to, size_t len, loff_t *offset)
{
	
	return 0;
}


static struct file_operations ircam_fops = {
	.owner = THIS_MODULE,
	.open = ircam_open,
	.write = ircam_write,
	.read = ircam_read,
	.release = ircam_release,
	.unlocked_ioctl = ircam_unlocked_ioctl,
};

/*----------------------------------------------------------------------------*/

static int __init ircam_init(void)
{
	int err,ret;
  	ret = alloc_chrdev_region(&ic_dev, 0, 1, DEV_NAME);
         if (ret< 0) {
		 printk("ircam  alloc_chrdev_region failed, %d", ret);
		return ret;
	}
	ircam_cdev= cdev_alloc();
	if (ircam_cdev == NULL) {
			 printk("ircam cdev_alloc failed");
			 ret = -ENOMEM;
			 goto EXIT;
	}
 	cdev_init(ircam_cdev, &ircam_fops);
	 ircam_cdev->owner = THIS_MODULE;
	 ret = cdev_add(ircam_cdev, ic_dev, 1);
	 if (ret < 0) {
		  printk("Attatch file ircam operation failed, %d", ret);
		 goto EXIT;
	 }
		 	 
	 ircam_class = class_create(THIS_MODULE, DEV_NAME);
			 if (IS_ERR(ircam_class)) {
				 printk("Failed to create class(ircam)!\n");
				 return PTR_ERR(ircam_class);
			 }
			 
	 ircam_device = device_create(ircam_class, NULL, ic_dev, NULL,DEV_NAME);
	 if (IS_ERR(ircam_device))
		 printk("Failed to create ircam_dev device\n");

printk("ircam  init  finish--------\n");

	return 0;

EXIT:
	if(ircam_cdev != NULL)
    {
        cdev_del(ircam_cdev);
        ircam_cdev = NULL;
    }

    unregister_chrdev_region(ic_dev, 1);
		 
	return ret;

}
/*----------------------------------------------------------------------------*/

static void __exit ircam_exit(void)
{
	int err;

	if(ircam_cdev != NULL)
	{
	    cdev_del(ircam_cdev);
	    ircam_cdev = NULL;
	}
	unregister_chrdev_region(ic_dev, 1);

}

module_init(ircam_init);
module_exit(ircam_exit);
MODULE_AUTHOR("Yongyida");
MODULE_DESCRIPTION("ircam driver");
MODULE_LICENSE("GPL");
