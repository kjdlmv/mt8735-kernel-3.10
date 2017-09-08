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
#include <linux/i2c.h>
#include <linux/sched.h>


#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/input.h>
#include <mach/mt_typedefs.h>

#include <y128_touchsensor.h>
#include <i2c_wrapper.h>

#define TOUCH  1
#define NO_TOUCH  0
#define TS_NAME  "touchsensor"
#define TS_SW_NAME "TouchSensor"
static char debuf[10];

#define TS_TASK_PERIOD_S                     0	
#define TS_TASK_PERIOD_NS                    3*100*1000*1000

static struct switch_dev touchsensor_dev;
struct input_dev *touchsensor_input_dev;

ktime_t ktime;

static struct hrtimer ts_kthread_timer;

static struct task_struct *thread = NULL;
static kal_bool ts_thread_timeout = KAL_FALSE;
static kal_bool ts_flag = KAL_FALSE;

struct i2c_client *i2c_client = NULL;

DECLARE_WAIT_QUEUE_HEAD(ts_thread_wq);

static void touchsensor_kthrd(void * argc);
static void ts_kthread_hrtimer_init(void);

static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DEFINE_MUTEX(i2c_access);

static const struct i2c_device_id ts_id[] = {{TS_NAME,0},{}};
static struct i2c_board_info __initdata wtc_i2c_ts={ I2C_BOARD_INFO(TS_NAME, (0x70>>1))};
static struct i2c_client *wtc_i2c_client = NULL;

static unsigned char WTC7514_CONFG[1] = {0};

static TOUCHDATA_T touch_record[TOUCH_MAX] = {KAL_FALSE};

kal_bool backlight_status = KAL_TRUE;


void commit_status(char *switch_name)
{
//	if(backlight_status == 1)
	if(1)
	{
		touchsensor_dev.name = switch_name;
		
		switch_set_state(&touchsensor_dev, TOUCH);
		
		touchsensor_dev.state = NO_TOUCH;
		touchsensor_dev.name = TS_NAME;
		
		TS_LOG("report %s\n", switch_name);

	}
	else
	{
		//printk("touchsensor don't commit_status because of backlight off--------%s----\n",switch_name);		
	}
	
}


static int ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	TS_ERR(" i2c probe\n");

	mt_set_gpio_mode(GPIO_TPPW_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_TPPW_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_TPPW_EN, 1);
	
	wtc_i2c_client = client;
	TS_LOG("sizeof(WTC7514_CONFG) = %d", sizeof(WTC7514_CONFG));
	wrapper_i2c_write_data(wtc_i2c_client, WTC7514_CONFG, sizeof(WTC7514_CONFG));
	msleep(50);
	thread = kthread_run(touchsensor_kthrd, 0,TS_NAME);
	ts_kthread_hrtimer_init();

	return 0;
}

static int ts_remove(struct i2c_client *client)
{

	TS_LOG(" i2c removed\n");
	hrtimer_cancel(&ts_kthread_timer);

	mt_set_gpio_out(GPIO_TPPW_EN, 0);
	return 0;
	
}

static int ts_detect (struct i2c_client *client, struct i2c_board_info *info) 
{
	strcpy(info->type, TS_NAME);	
	return 0;
}

static struct i2c_driver ts_device_driver = {
  	.driver = {
		.owner	= THIS_MODULE,
	 	.name 	= TS_NAME,
  	},
  	.probe 	= ts_probe,
  	.remove 	= ts_remove,
  	.id_table 	= ts_id,
  	.detect 	= ts_detect,
};
static void touchsensor_getmap(unsigned char *data)
{
	int i = 0, tmp = 0;
	int sw_data_sum = data[0]*256 + data[1];
	tmp = sw_data_sum;
	for(i = 0; i < TOUCH_MAX; i++)
	{
		if(tmp & 0x1)
		{
			if(touch_record[i].down == KAL_FALSE)
			{
				touch_record[i].valid = KAL_TRUE;
			}
			touch_record[i].down = KAL_TRUE;
		}
		else
		{
			touch_record[i].valid = KAL_FALSE;
			touch_record[i].down = KAL_FALSE;
		}
			
		tmp = tmp >> 1;
	}
}

static void touchsensor_report(void)
{
	int i = 0;
	for(i = 0; i < TOUCH_MAX; i++)
	{
		if(touch_record[i].valid)
		{
			commit_status(TouchMap[touch_pos[i]]);
			touch_record[i].valid = KAL_FALSE;
			msleep(10);
		}
	}
}

static void touchsensor_func(void)
{
	printk("delay_hrtimer_func	finish-------------\n");
	return 0;
}

static void touchsensor_get_info(void)
{
	unsigned char touchsensor_data[2];
	wrapper_i2c_read_data(wtc_i2c_client, touchsensor_data, sizeof(touchsensor_data));
	touchsensor_getmap(touchsensor_data);
	touchsensor_report();
	TS_LOG("read data[0] %d, read data[1] %d\n", touchsensor_data[0], touchsensor_data[1]);
	//TS_ERR("read data[0] %d, read data[1] %d\n", touchsensor_data[0], touchsensor_data[1]);
}

static void touchsensor_kthrd(void * argc)
{
	//allow_signal(SIGKILL);
	
	while(1)
	{
		//set_current_state(TASK_INTERRUPTIBLE);
		//每隔1s执行1次，然后睡眠，可被信号提前唤醒
		//schedule_timeout_interruptible(1*HZ);
		//如果收到信号（SIGKILL），就退出
		//if(signal_pending(current))
			//break;
		wait_event(ts_thread_wq, (ts_thread_timeout == KAL_TRUE));
		ts_thread_timeout = KAL_FALSE;
	   	touchsensor_get_info();
		
		ktime = ktime_set(TS_TASK_PERIOD_S, TS_TASK_PERIOD_NS);
		hrtimer_forward_now(&ts_kthread_timer, ktime);
		hrtimer_start(&ts_kthread_timer, ktime, HRTIMER_MODE_REL);
	}
	
	//set_current_state(TASK_RUNNING);
	return 0;

}

static int touch_event_handler(void *unused)
{
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	
	sched_setscheduler(current, SCHED_RR, &param);
	do{
		set_current_state(TASK_INTERRUPTIBLE); 
		wait_event_interruptible(waiter,ts_flag!=0);						 
		ts_flag = 0;			 
		set_current_state(TASK_RUNNING);
	}while(!kthread_should_stop());
	return 0;
}

static void ts_eint_interrupt_handler()
{
	ts_flag = 1;
	wake_up_interruptible(&waiter);
}

enum hrtimer_restart ts_kthread_hrtimer_func(struct hrtimer *timer)
{
	TS_LOG("wake_up ts_thread_wq\n");
	ts_thread_timeout = KAL_TRUE;
	wake_up(&ts_thread_wq);

	return HRTIMER_NORESTART;
}

static void ts_kthread_hrtimer_init(void)
{
	ktime = ktime_set(TS_TASK_PERIOD_S, TS_TASK_PERIOD_NS);
	hrtimer_init(&ts_kthread_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);  //CLOCK_MONOTONIC
	ts_kthread_timer.function = ts_kthread_hrtimer_func;
	hrtimer_start(&ts_kthread_timer, ktime, HRTIMER_MODE_REL);
}

static int __init touchsensor_init(void)
{

	int err = 0;

	TS_ERR("init start\n");

	touchsensor_dev.name = TS_NAME;
	touchsensor_dev.index = 0;
	touchsensor_dev.state = 0;
	 
	switch_dev_register(&touchsensor_dev);
/*********************************************************/
	touchsensor_input_dev = input_allocate_device();	//分配设备
	__set_bit(EV_SYN, touchsensor_input_dev->evbit);  //注册设备支持event类型
	__set_bit(EV_KEY, touchsensor_input_dev->evbit);
	
	__set_bit(KEY_VOLUMEUP, touchsensor_input_dev->keybit);   
	__set_bit(KEY_VOLUMEDOWN, touchsensor_input_dev->keybit);  

	touchsensor_input_dev->name =TS_NAME;//
	err = input_register_device(touchsensor_input_dev);				
	if (err) {
		TS_ERR("register input device failed (%d)\n", err);
		input_free_device(touchsensor_input_dev);
		return err;
	}
/*********************************************************/
	i2c_register_board_info(TS_I2C_NUMBER, &wtc_i2c_ts, 1);

	if(i2c_add_driver(&ts_device_driver) < 0)
	{
		TS_ERR("add driver failed\n");
	 	return -1;
	}

	TS_ERR("init finish\n");
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
MODULE_DESCRIPTION("touchsensor driver");
MODULE_LICENSE("GPL");

MODULE_AUTHOR("yongyida@yongyida.com>");
MODULE_DESCRIPTION("touchsersor driver");
MODULE_LICENSE("GPL");

