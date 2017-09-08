#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/leds-mt65xx.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <linux/device.h>   /*class_create*/ 
#include <asm/uaccess.h>	/* copy_*_user */
#include <linux/kthread.h>

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>

#include <linux/i2c.h>
#include <mach/mt_gpio.h>

#include "pir_sensor.h"
#include <y128_touchsensor.h>
static void pir_sensor_eint_handler(void);

//#define USE_ENT  1

#define PS_NAME  "pirsensor"
#define PS_SW_NAME "PirSensor"
static char debuf[10];

#define PS_TASK_PERIOD_S                     0	
#define PS_TASK_PERIOD_NS                    3*100*1000*1000

static struct task_struct *thread = NULL;

static kal_bool people_in = KAL_FALSE;
static kal_bool status_changed = KAL_FALSE;
static char *commit_str;
/***************************************************************************/
static void pirsensor_get_info(void)
{
	char sta = mt_get_gpio_in(PIR_SENSOR_EINT_PIN);

	if(people_in == KAL_FALSE)
	{
		if(sta == 0)
		{
			status_changed = KAL_FALSE;
		}else
		{
			status_changed = KAL_TRUE;
			commit_str = "pir_in";
			people_in = KAL_TRUE;
		}
	}
	else
	{
		if(sta == 0)
		{
			status_changed = KAL_TRUE;
			commit_str = "pir_out";
			people_in = KAL_FALSE;
			
		}
		else
		{
			status_changed = KAL_FALSE;
		}
	}
	PS_LOG("status_changed = %d, people_in = %d\n", status_changed, people_in);

}
static void pirsensor_report(void)
{
	if(status_changed)
		commit_status(commit_str);
	PS_LOG("commit status: %s\n", commit_str);
}

static void pirsensor_kthrd(void * argc)
{
	allow_signal(SIGKILL);
	
	while(1)
	{
		set_current_state(TASK_INTERRUPTIBLE);
		//每隔1s执行1次，然后睡眠，可被信号提前唤醒
		schedule_timeout_interruptible(HZ*1);
		//如果收到信号（SIGKILL），就退出
		if(signal_pending(current))
			break;
/*		
		wait_event(ts_thread_wq, (ts_thread_timeout == KAL_TRUE));
		ts_thread_timeout = KAL_FALSE;
*/		
	   	pirsensor_get_info();
		pirsensor_report();
/*		
		ktime = ktime_set(TS_TASK_PERIOD_S, TS_TASK_PERIOD_NS);
		hrtimer_forward_now(&ts_kthread_timer, ktime);
		hrtimer_start(&ts_kthread_timer, ktime, HRTIMER_MODE_REL);
*/
	}
	
	set_current_state(TASK_RUNNING);
	return 0;

}
static int pir_sensor_probe(struct platform_device *pdev)
{
	int ret;
#ifdef USE_ENT
	mt_set_gpio_mode(PIR_SENSOR_EINT_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(PIR_SENSOR_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(PIR_SENSOR_EINT_PIN, FALSE);

	mt_eint_set_hw_debounce(CUST_EINT_CHR_STAT_NUM, 100);
	mt_eint_registration(CUST_EINT_CHR_STAT_NUM, CUST_EINTF_TRIGGER_RISING, pir_sensor_eint_handler, 0);			
	mt_eint_unmask(CUST_EINT_CHR_STAT_NUM);
#else
	mt_set_gpio_mode(PIR_SENSOR_EINT_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(PIR_SENSOR_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(PIR_SENSOR_EINT_PIN, FALSE);

	thread = kthread_run(pirsensor_kthrd, 0,PS_NAME);
#endif
	PS_LOG("probe");
	return ret;
}


static int pir_sensor_remove(struct platform_device *pdev)
{
    return 0;
}

static int pir_sensor_suspend(struct platform_device *pdev, pm_message_t mesg)
{
/*
	mt_set_gpio_mode(PIR_SENSOR_EINT_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(PIR_SENSOR_EINT_PIN, GPIO_DIR_IN);

	mt_eint_set_hw_debounce(CUST_EINT_CHR_STAT_NUM, 100);
	mt_eint_registration(CUST_EINT_CHR_STAT_NUM, CUST_EINTF_TRIGGER_RISING, pir_sensor_eint_handler, 0);			
	mt_eint_unmask(CUST_EINT_CHR_STAT_NUM);
*/
	PS_LOG("suspend");
    return 0;
}
static int pir_sensor_shutdown(struct platform_device *pdev)
{
	return 0;
}

static int pir_sensor_resume(struct platform_device *pdev)
{
/*
	mt_set_gpio_mode(PIR_SENSOR_EINT_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(PIR_SENSOR_EINT_PIN, GPIO_DIR_IN);

	mt_eint_set_hw_debounce(CUST_EINT_CHR_STAT_NUM, 100);
	mt_eint_registration(CUST_EINT_CHR_STAT_NUM, CUST_EINTF_TRIGGER_RISING, pir_sensor_eint_handler, 0);			
	mt_eint_unmask(CUST_EINT_CHR_STAT_NUM);
*/
	PS_LOG("resume");
    return 0;
}




static struct platform_device pir_sensor_device = {
	.name = "pir_sensor",
	.id = -1
};


// platform structure
static struct platform_driver pir_sensor_driver = {
    .probe		= pir_sensor_probe,
    .remove	= pir_sensor_remove,
    .shutdown = pir_sensor_shutdown,
    .suspend	= pir_sensor_suspend,
    .resume	= pir_sensor_resume,
    .driver		= {
        .name	= "pir_sensor",
        .owner	= THIS_MODULE,
    }
};

static void pir_sensor_eint_handler(void)
{
	PS_LOG("interrupt happened\n");
	commit_status("pir");
	mt_eint_unmask(CUST_EINT_CHR_STAT_NUM);
}

static int __init pir_sensor_init(void)
{
	int ret;
	
	PS_LOG("init start\n");

	mt_set_gpio_mode(PIR_SENSOR_PWEREN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(PIR_SENSOR_PWEREN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(PIR_SENSOR_PWEREN_PIN, 1);
	
	mt_set_gpio_mode((GPIO0 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_dir((GPIO0 | 0x80000000), GPIO_DIR_IN);
/*	
	//mt_set_gpio_pull_select((GPIO0 | 0x80000000), GPIO_PULL_DOWN);
	mt_set_gpio_pull_enable((GPIO0 | 0x80000000), false);				
	mt_eint_set_hw_debounce(CUST_EINT_CHR_STAT_NUM, 100);
	mt_eint_registration(CUST_EINT_CHR_STAT_NUM, CUST_EINTF_TRIGGER_RISING, pir_sensor_eint_handler, 0);			
	mt_eint_unmask(CUST_EINT_CHR_STAT_NUM);
*/	
	ret = platform_device_register(&pir_sensor_device);
	if (ret)
		PS_ERR("platform_device_register");
	
	ret = platform_driver_register(&pir_sensor_driver);

	if (ret) {
		PS_ERR("platform_driver_register\n");
		return ret;
	}

	PS_LOG("init finish\n");



	return ret;
}

static void __exit pir_sensor_exit(void)
{
	PS_LOG("exit");
}

late_initcall_sync(pir_sensor_init);
module_exit(pir_sensor_exit);

MODULE_AUTHOR("MediaTek Inc.");
MODULE_DESCRIPTION("pir_sensor driver for MediaTek");
MODULE_LICENSE("GPL");