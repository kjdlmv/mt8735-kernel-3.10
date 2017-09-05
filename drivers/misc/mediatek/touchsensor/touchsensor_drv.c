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


#define TOUCH  1
#define NO_TOUCH  0
#define TP_NAME  "touchsensor"
static char debuf[10];

static struct switch_dev touchsensor_dev;
static struct hrtimer timer;
static struct hrtimer dance_timer;
static unsigned long left_time,right_time;
static struct task_struct *thread = NULL;

extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt_eint_print_status(void);
extern void aw2013_breath_all(int led0,int led1,int led2);

struct input_dev *touchsensor_input_dev;


static void commit_status(char *switch_name)
{
	touchsensor_dev.name=switch_name;
	
	switch_set_state(&touchsensor_dev, TOUCH);
	
	touchsensor_dev.state=NO_TOUCH;
	touchsensor_dev.name = TP_NAME;

	printk("touchsensor commit_status--------%s----\n",switch_name);
}

static void touchsensor_eint76_func(void)
{
	static unsigned long time = 0;
	
	if((jiffies - time)/HZ > 5 || time == 0)
	{
		printk("daviekuo: in head touch %s", __func__);
		commit_status("head");	//
		time = jiffies;
	}

 	mt_eint_unmask(CUST_EINT_OFN_NUM);
}
static void touchsensor_eint77_func(void)
{
	static unsigned long time = 0;
	if((jiffies - time)/HZ > 5 || time == 0)
	{
		commit_status("head");	//
		time = jiffies;
	}
	printk("daviekuo: %s", __func__);
 	mt_eint_unmask(CUST_EINT_NFC_NUM);
}

static void touchsensor_eint11_func(void)
{
	commit_status("vol+");	//volume +
	input_report_key(touchsensor_input_dev, KEY_VOLUMEUP,1);
	input_sync(touchsensor_input_dev);
	input_report_key(touchsensor_input_dev, KEY_VOLUMEUP,0);
	input_sync(touchsensor_input_dev);

	mt_eint_unmask(CUST_EINT_HALL_3_NUM);
}

static void touchsensor_eint12_func(void)
{
	commit_status("vol-");	//volume -
	
	input_report_key(touchsensor_input_dev, KEY_VOLUMEDOWN,1);
	input_sync(touchsensor_input_dev);
	input_report_key(touchsensor_input_dev, KEY_VOLUMEDOWN,0);
	input_sync(touchsensor_input_dev);

	mt_eint_unmask(CUST_EINT_HALL_4_NUM);
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
					
	mt_eint_registration(CUST_EINT_OFN_NUM, CUST_EINTF_TRIGGER_LOW, touchsensor_eint76_func, 0);		
	mt_eint_set_hw_debounce(CUST_EINT_OFN_NUM, 300);
	mt_eint_unmask(CUST_EINT_OFN_NUM);
*/
	mt_set_gpio_dir((GPIO77 | 0x80000000), GPIO_DIR_IN);
	mt_set_gpio_mode((GPIO77 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_pull_enable((GPIO77 | 0x80000000), FALSE);

	mt_eint_registration(CUST_EINT_NFC_NUM, CUST_EINTF_TRIGGER_LOW, touchsensor_eint77_func, 0);								
	mt_eint_set_hw_debounce(CUST_EINT_NFC_NUM, 300);
	mt_eint_unmask(CUST_EINT_NFC_NUM);

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
/*
	ktime_t ktime;
	ktime = ktime_set(25, 0);	 //60s later  register ext_int
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer.function = touchsensor_func;
	hrtimer_start(&timer, ktime, HRTIMER_MODE_REL);
*/
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

MODULE_AUTHOR("yongyida@yongyida.com>");
MODULE_DESCRIPTION("touchsersor driver");
MODULE_LICENSE("GPL");
