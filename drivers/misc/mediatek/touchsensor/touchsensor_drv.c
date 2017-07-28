#include <linux/switch.h>

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>

#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/interrupt.h>

#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/input.h>

#include <misc.h>

#define TOUCH  1
#define NO_TOUCH  0
#define TP_NAME  "touchsensor"
static char debuf[10];

static struct switch_dev touchsensor_dev;
ktime_t ktime;
static struct hrtimer timer;
static struct hrtimer dance_timer;
static unsigned long left_time,right_time,dis_time;
static struct task_struct *thread = NULL;
static int timer_sta = 0;

extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt_eint_print_status(void);
extern void aw2013_breath_all(int led0,int led1,int led2);

extern char backlight_status;


struct input_dev *touchsensor_input_dev;


static void commit_status(char *switch_name)
{
	if(backlight_status == 1)
	{
		touchsensor_dev.name=switch_name;
		
		switch_set_state(&touchsensor_dev, TOUCH);
		
		touchsensor_dev.state=NO_TOUCH;
		touchsensor_dev.name = TP_NAME;
		
		printk("touchsensor commit_status because of backlight on--------%s----\n",switch_name);

	}
	else
		printk("touchsensor don't commit_status because of backlight off--------%s----\n",switch_name);		
}

static void touchsensor_eint76_func(void)
{
	static unsigned long time = 0;
	int gpio_read = 0;
/*	
	if((jiffies - time)/HZ > 4 || time == 0)
	{
		printk("daviekuo: in head touch %s", __func__);
		//commit_status("head");	//
		commit_status("t_back_center");	//
		time = jiffies;
	}
*/
	mdelay(15);
	gpio_read = mt_get_gpio_in((GPIO76 | 0x80000000));

	if(gpio_read == 0)
	{
		commit_status("yyd7");
		printk("daviekuo: t_back_center\n");
	}	
	printk("daviekuo: %s\n", __func__);
 	mt_eint_unmask(CUST_EINT_OFN_NUM);
}
static void touchsensor_eint77_func(void)
{
	static unsigned long time = 0;
	int gpio_read = 0;
/*	
	if((jiffies - time)/HZ > 2 || time == 0)
	{
		//commit_status("head");	//
		commit_status("t_back_head");	//
		time = jiffies;
	}
*/	
	mdelay(15);
	gpio_read = mt_get_gpio_in((GPIO77 | 0x80000000));
	if(gpio_read == 0)
	{
		commit_status("t_head");
		//commit_status("wakeup");
		printk("daviekuo: wakeup\n");
	}
	printk("daviekuo: %s\n", __func__);
 	mt_eint_unmask(CUST_EINT_NFC_NUM);
}
enum hrtimer_restart double_touch_commit(struct hrtimer *timer)
{
	if((mt_get_gpio_in((GPIO11 | 0x80000000)) == 0) && (0 == mt_get_gpio_in((GPIO12 | 0x80000000))))
	{
		commit_status("dance");
		
	}
	timer_sta = 0;	
	printk("daviekuo %s\n", __func__);
	return HRTIMER_NORESTART;
}
static void touchsensor_eint11_func(void)
{
	right_time = jiffies;

/*	
	if(right_time - left_time >= 0)
	{
		dis_time = right_time - left_time;
	}else
		{
		dis_time = left_time - right_time;
	}
*/	

	commit_status("yyd6");	//volume +
	//commit_status("volup");	//volume +
	input_report_key(touchsensor_input_dev, KEY_VOLUMEUP,1);
	input_sync(touchsensor_input_dev);
	input_report_key(touchsensor_input_dev, KEY_VOLUMEUP,0);
	input_sync(touchsensor_input_dev);

	dis_time = abs(right_time - left_time);
	if(dis_time < HZ)
	{
		right_time = 0;
		left_time = 0;
		//mdelay(10);	
		if(timer_sta == 0)
		{
			timer_sta = 1;
			hrtimer_forward_now(&timer, ktime_set(2, 0));
			hrtimer_start(&timer, ktime, HRTIMER_MODE_REL); 
		}
		
		//commit_status("yyd2");	
		printk("daviekuo: right dance\n");
	}

	printk("daviekuo: volup %s\n", __func__);
	mt_eint_unmask(CUST_EINT_HALL_3_NUM);
}

static void touchsensor_eint12_func(void)
{
	left_time = jiffies;

	commit_status("yyd5");	//volume -
//	commit_status("voldown");	//volume -
	input_report_key(touchsensor_input_dev, KEY_VOLUMEDOWN,1);
	input_sync(touchsensor_input_dev);
	input_report_key(touchsensor_input_dev, KEY_VOLUMEDOWN,0);
	input_sync(touchsensor_input_dev);
	
	dis_time = abs(right_time - left_time);
	if(dis_time < HZ)
	{
		left_time = 0;
		right_time = 0;
		//mdelay(10);
		if(timer_sta == 0)
		{
			timer_sta = 1;
			hrtimer_forward_now(&timer, ktime_set(2, 0));
			hrtimer_start(&timer, ktime, HRTIMER_MODE_REL); 
		}
		
		//commit_status("yyd2");		
		printk("daviekuo: left dance\n");
	}	
	printk("daviekuo: voldown %s\n", __func__);
	mt_eint_unmask(CUST_EINT_HALL_4_NUM);
}

static void touchsensor_eint8_func(void)
{
	
	static unsigned long time = 0;
	
	if((jiffies - time)/HZ > 2 || time == 0)
	{
		commit_status("yyd3");	//
		//commit_status("entocn");	//
		time = jiffies;
	}
	printk("daviekuo: entocn %s\n", __func__);
 	mt_eint_unmask(CUST_EINT_COMBO_ALL_NUM);

/*
	commit_status("vol+");	//volume +
	input_report_key(touchsensor_input_dev, KEY_VOLUMEUP,1);
	input_sync(touchsensor_input_dev);
	input_report_key(touchsensor_input_dev, KEY_VOLUMEUP,0);
	input_sync(touchsensor_input_dev);
	mt_eint_unmask(CUST_EINT_CHR_STAT_NUM);
*/
}
static void touchsensor_eint10_func(void)
{
	static unsigned long time = 0;
	if((jiffies - time)/HZ > 2 || time == 0)
	{
		commit_status("yyd4");	//
		//commit_status("cntoen");	//
		time = jiffies;
	}
	printk("daviekuo: cntoen %s\n", __func__);
 	mt_eint_unmask(CUST_EINT_COMBO_BGF_NUM);
}

enum hrtimer_restart commit_delay_hrtimer_func(struct hrtimer *timer)
{
	if((jiffies -right_time) >4*HZ)right_time=0;
	if((jiffies -left_time) >4*HZ)left_time=0;

	if( (jiffies -right_time) >3*HZ && (jiffies -right_time) <4*HZ && (jiffies -left_time) >3*HZ && (jiffies -left_time) <4*HZ && abs(left_time-right_time)<10)	
		commit_status("dance");
	hrtimer_forward_now(&dance_timer, ktime_set(1, 0));	
	

	return HRTIMER_RESTART;
}

//enum hrtimer_restart delay_hrtimer_func(struct hrtimer *timer)
void touchsensor_func(void)
{

	mt_set_gpio_dir((GPIO11 | 0x80000000), GPIO_DIR_IN);
	mt_set_gpio_mode((GPIO11 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_pull_enable((GPIO11 | 0x80000000), FALSE);
					
			
	mt_eint_set_hw_debounce(CUST_EINT_HALL_3_NUM, 80);
	mt_eint_registration(CUST_EINT_HALL_3_NUM, CUST_EINTF_TRIGGER_FALLING, touchsensor_eint11_func, 0); 
	mt_eint_unmask(CUST_EINT_HALL_3_NUM);

	mt_set_gpio_dir((GPIO12 | 0x80000000), GPIO_DIR_IN);
	mt_set_gpio_mode((GPIO12 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_pull_enable((GPIO12 | 0x80000000), FALSE);

	mt_eint_set_hw_debounce(CUST_EINT_HALL_4_NUM, 80);
	mt_eint_registration(CUST_EINT_HALL_4_NUM, CUST_EINTF_TRIGGER_FALLING, touchsensor_eint12_func, 0); 								
	mt_eint_unmask(CUST_EINT_HALL_4_NUM);
/*
	mt_set_gpio_dir((GPIO76 | 0x80000000), GPIO_DIR_IN);
	mt_set_gpio_mode((GPIO76 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_pull_enable((GPIO76 | 0x80000000), FALSE);
				
	mt_eint_set_hw_debounce(CUST_EINT_OFN_NUM, 200);
	mt_eint_registration(CUST_EINT_OFN_NUM, CUST_EINTF_TRIGGER_FALLING, touchsensor_eint76_func, 0);		
	mt_eint_unmask(CUST_EINT_OFN_NUM);

*/
	mt_set_gpio_dir((GPIO77 | 0x80000000), GPIO_DIR_IN);
	mt_set_gpio_mode((GPIO77 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_pull_enable((GPIO77 | 0x80000000), FALSE);
	
/*		
	mt_set_gpio_pull_enable((GPIO77 | 0x80000000), GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select((GPIO77 | 0x80000000), GPIO_PULL_UP);
*/
	mt_eint_set_hw_debounce(CUST_EINT_NFC_NUM, 200);
	mt_eint_registration(CUST_EINT_NFC_NUM, CUST_EINTF_TRIGGER_FALLING, touchsensor_eint77_func, 0);								
	mt_eint_unmask(CUST_EINT_NFC_NUM);

	mt_set_gpio_dir((GPIO8 | 0x80000000), GPIO_DIR_IN);
	mt_set_gpio_mode((GPIO8 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_pull_enable((GPIO8 | 0x80000000), FALSE);

	mt_eint_set_hw_debounce(CUST_EINT_COMBO_ALL_NUM, 80);
	mt_eint_registration(CUST_EINT_COMBO_ALL_NUM, CUST_EINTF_TRIGGER_FALLING, touchsensor_eint8_func, 0);								
	mt_eint_unmask(CUST_EINT_COMBO_ALL_NUM);

	
	mt_set_gpio_dir((GPIO10 | 0x80000000), GPIO_DIR_IN);
	mt_set_gpio_mode((GPIO10 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_pull_enable((GPIO10 | 0x80000000), FALSE);

	mt_eint_set_hw_debounce(CUST_EINT_COMBO_BGF_NUM, 80);
	mt_eint_registration(CUST_EINT_COMBO_BGF_NUM, CUST_EINTF_TRIGGER_FALLING, touchsensor_eint10_func, 0);								
	mt_eint_unmask(CUST_EINT_COMBO_BGF_NUM);

	printk("delay_hrtimer_func	finish-------------\n");

	return 0;
}


static int __init touchsensor_init(void)
{

	int err = 0;

	touchsensor_dev.name = TP_NAME;
	touchsensor_dev.index = 0;
	touchsensor_dev.state = 0;
	
	switch_dev_register(&touchsensor_dev);
/*********************************************************/
	touchsensor_input_dev = input_allocate_device();	//分配设备
	__set_bit(EV_SYN, touchsensor_input_dev->evbit);  //注册设备支持event类型
	__set_bit(EV_KEY, touchsensor_input_dev->evbit);
	
	__set_bit(KEY_VOLUMEUP, touchsensor_input_dev->keybit);   
	__set_bit(KEY_VOLUMEDOWN, touchsensor_input_dev->keybit);  

	touchsensor_input_dev->name ="TouchSensor";//
	err = input_register_device(touchsensor_input_dev);				
	if (err) {
		printk( "TouchSensor register input device failed (%d)\n", err);
		input_free_device(touchsensor_input_dev);
		return err;
	}
/*********************************************************/
/*
	mt_set_gpio_mode((GPIO78 | 0x80000000), GPIO_MODE_00); 
	mt_set_gpio_dir((GPIO78 | 0x80000000), GPIO_DIR_OUT) ;
	mt_set_gpio_out((GPIO78 | 0x80000000), 1); 
*/

	ktime = ktime_set(2, 0);	 //60s later  register ext_int
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer.function = double_touch_commit;
	//hrtimer_start(&timer, ktime, HRTIMER_MODE_REL);

	touchsensor_func();
	printk("touchsensor       init      finish-------------\n");
	return  0;
}

static void __exit touchsensor_exit(void)
{
	 switch_dev_unregister(&touchsensor_dev);
	 input_unregister_device(touchsensor_input_dev);
}	
	
module_init(touchsensor_init);
module_exit(touchsensor_exit);

MODULE_AUTHOR("Yongyida");
MODULE_DESCRIPTION("motor driver");
MODULE_LICENSE("GPL");

MODULE_AUTHOR("yongyida@yongyida.com>");
MODULE_DESCRIPTION("touchsersor driver");
MODULE_LICENSE("GPL");
