#include <linux/module.h>
#include <linux/platform_device.h>
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
#include <linux/earlysuspend.h>

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>

#include <mach/battery_common.h>
#include <mach/mt_boot_common.h>
#include <misc.h>
/*----------------------------------------------------------------------------*/
#include<linux/wakelock.h>
#define CHG_DET_EN_PIN (GPIO6 | 0x80000000)
#define CHG_DET_PIN    (GPIO120 | 0x80000000)
static struct wake_lock m_lock;

#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_EARLYSUSPEND

static void m_suspend( struct early_suspend *h )
{
//----------------li fei-----------------

		mt_set_gpio_out((GPIO127|0x80000000),0);
		mt_set_gpio_out((GPIO168|0x80000000),0);
		//mt_set_gpio_out((GPIO87|0x80000000),0);
		mt_set_gpio_out((GPIO129|0x80000000),0);
		mt_set_gpio_out((GPIO94|0x80000000),0);
		mt_set_gpio_out((GPIO2|0x80000000),0);

	//-----------------------------------

	mt_set_gpio_out(BLUE_LED_PIN, 0); 
}

static void m_resume( struct early_suspend *h )
{
//----------------li fei-----------------
	mt_set_gpio_out((GPIO127|0x80000000),1);
	mt_set_gpio_out((GPIO168|0x80000000),1);
	//mt_set_gpio_out((GPIO87|0x80000000),1);
	mt_set_gpio_out((GPIO129|0x80000000),1);
	mt_set_gpio_out((GPIO94|0x80000000),1);
	mt_set_gpio_out((GPIO2|0x80000000),1);

//-----------------------------------
	mt_set_gpio_out(BLUE_LED_PIN, 1);	
}

static struct early_suspend misc_early_suspend_handler = {
	.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 1,
	.suspend = m_suspend,
	.resume = m_resume,
};

#endif
#endif

static int misc_probe(struct platform_device *pdev)
{
	int ret = 0;
	
	BOOTMODE bootmode = NORMAL_BOOT;
	//mt_set_gpio_pull_enable(LD0_3V3_PIN, GPIO_PULL_ENABLE);	

	wake_lock_init(&m_lock, WAKE_LOCK_SUSPEND, "misc"); //davie: forbid deep sleep for 5mic
	wake_lock(&m_lock);
	
	printk("misc setting  init	finish--------\n");
	//Motor
	mt_set_gpio_mode(MOTOR_RST_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 1);
	
	mt_set_gpio_mode(MOTOR_BOOT0_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 0);
	
	mt_set_gpio_mode(MOTOR_POWEN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 1);

//	mt_set_gpio_dir(CHG_CTL_PIN, GPIO_DIR_OUT);
//	mt_set_gpio_out(CHG_CTL_PIN, 0);		

	//Easy home
	mt_set_gpio_mode(MOTOR_ZNJJPOW_PIN, GPIO_MODE_00);	
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 1);	
	
	//Charger
	//mt_set_gpio_mode(CHG_EN_PIN, GPIO_MODE_00);
	//mt_set_gpio_dir(CHG_EN_PIN, GPIO_DIR_OUT);
	//mt_set_gpio_out(CHG_EN_PIN, 0);	
	
	
	mt_set_gpio_mode(LD0_3V3_PIN, GPIO_MODE_00); 
	mt_set_gpio_dir(LD0_3V3_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LD0_3V3_PIN, 1);
	
	mt_set_gpio_mode(BLUE_LED_PIN, GPIO_MODE_00); 
	//mt_set_gpio_pull_enable(LD0_3V3_PIN, GPIO_PULL_ENABLE);	
	mt_set_gpio_dir(BLUE_LED_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(BLUE_LED_PIN, 1);
	
	
	mt_set_gpio_mode(CHG_DET_EN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(CHG_DET_EN_PIN, GPIO_DIR_OUT);
	
	mt_set_gpio_dir(CHG_DET_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(CHG_DET_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(CHG_DET_PIN, GPIO_PULL_UP);	
  
	if(KERNEL_POWER_OFF_CHARGING_BOOT == bootmode || LOW_POWER_OFF_CHARGING_BOOT == bootmode)
	{
		mt_set_gpio_out(CHG_DET_EN_PIN, 0);
		MISC_LOG("CHG_DET_EN_PIN, 0\n");
	}
	else
	{
		mt_set_gpio_out(CHG_DET_EN_PIN, 1);
		MISC_LOG("CHG_DET_EN_PIN,1\n");
	}
	
	MISC_LOG("probe	finish--------\n");

	return ret;

EXIT:		 
	return -1;


}


static int misc_remove(struct platform_device *pdev)
{
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 0);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 1);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 0);
	
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 0);	
		
	return 0;

}

static int misc_suspend(struct platform_device *pdev, pm_message_t mesg)
{

    return 0;
}
static int misc_shutdown(struct platform_device *pdev)
{
	return 0;
}

static int misc_resume(struct platform_device *pdev)
{

    return 0;
}


static struct platform_device misc_device = {
	.name = "misc",
	.id = -1
};


// platform structure
static struct platform_driver misc_driver = {
    .probe		= misc_probe,
    .remove	= misc_remove,
    .shutdown = misc_shutdown,
    .suspend	= misc_suspend,
    .resume	= misc_resume,
    .driver		= {
        .name	= "misc",
        .owner	= THIS_MODULE,
    }
};
/*

static int __init misc_init(void)
{
	int ret = 0;
	BOOTMODE bootmode = NORMAL_BOOT;
	//mt_set_gpio_pull_enable(LD0_3V3_PIN, GPIO_PULL_ENABLE);	
	
	printk("misc setting  init  finish--------\n");
	//Motor
	mt_set_gpio_mode(MOTOR_RST_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_RST_PIN, 1);
	
	mt_set_gpio_mode(MOTOR_BOOT0_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_BOOT0_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_BOOT0_PIN, 0);
	
	mt_set_gpio_mode(MOTOR_POWEN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(MOTOR_POWEN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_POWEN_PIN, 1);

//	mt_set_gpio_dir(CHG_CTL_PIN, GPIO_DIR_OUT);
//	mt_set_gpio_out(CHG_CTL_PIN, 0);		

	//Easy home
	mt_set_gpio_mode(MOTOR_ZNJJPOW_PIN, GPIO_MODE_00);	
	mt_set_gpio_dir(MOTOR_ZNJJPOW_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(MOTOR_ZNJJPOW_PIN, 1);	
	
	//Charger
	//mt_set_gpio_mode(CHG_EN_PIN, GPIO_MODE_00);
	//mt_set_gpio_dir(CHG_EN_PIN, GPIO_DIR_OUT);
	//mt_set_gpio_out(CHG_EN_PIN, 0);	
	
	
	mt_set_gpio_mode(LD0_3V3_PIN, GPIO_MODE_00); 
	mt_set_gpio_dir(LD0_3V3_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LD0_3V3_PIN, 1);
	
	mt_set_gpio_mode(BLUE_LED_PIN, GPIO_MODE_00); 
	//mt_set_gpio_pull_enable(LD0_3V3_PIN, GPIO_PULL_ENABLE);	
	mt_set_gpio_dir(BLUE_LED_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(BLUE_LED_PIN, 1);
	
	
		mt_set_gpio_mode(CHG_DET_EN_PIN, GPIO_MODE_00);
		mt_set_gpio_dir(CHG_DET_EN_PIN, GPIO_DIR_OUT);
	
	mt_set_gpio_dir(CHG_DET_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(CHG_DET_PIN, GPIO_PULL_ENABLE);
  mt_set_gpio_pull_select(CHG_DET_PIN, GPIO_PULL_UP);	
  
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
	
	printk("init  finish--------\n");

	return ret;

EXIT:		 
	return -1;
}




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
}
*/

static int __init misc_init(void)
{
	int ret;

	ret = platform_device_register(&misc_device);
	if (ret)
		MISC_ERR("device register : %d\n", ret);
	
	ret = platform_driver_register(&misc_driver);

	if (ret) {
		MISC_ERR("driver register: %d\n", ret);
		return ret;
	}
	#ifdef CONFIG_HAS_EARLYSUSPEND
	#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&misc_early_suspend_handler);
	#endif
	#endif
	
	return ret;
}

static void __exit misc_exit(void)
{
	platform_driver_unregister(&misc_driver);
	wake_lock_destroy(&m_lock);
}

module_init(misc_init);
module_exit(misc_exit);
MODULE_AUTHOR("Yongyida");
MODULE_DESCRIPTION("misc driver");
MODULE_LICENSE("GPL");

