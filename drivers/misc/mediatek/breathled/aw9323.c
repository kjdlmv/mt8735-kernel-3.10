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
#include <linux/earlysuspend.h>

#include <linux/i2c.h>
#include <mach/mt_gpio.h>
#include <linux/kthread.h>

#include <i2c_wrapper.h>
#include <aw9323.h>

#define BLD_NAME "breathled"

static DEFINE_MUTEX(bld_access);

static ktime_t ktime;

static struct hrtimer bld_kthread_timer;

static struct task_struct *thread = NULL;
static kal_bool bld_thread_timeout = KAL_FALSE;
static kal_bool bld_flag = KAL_FALSE;

static struct i2c_client *i2c_client = NULL;

DECLARE_WAIT_QUEUE_HEAD(bld_thread_wq);

static void breathled_kthrd(void * argc);
static void bld_kthread_hrtimer_init(void);

static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DEFINE_MUTEX(i2c_access);



I2C_GPIO_T bld_i2c_gpio_l;
I2C_GPIO_T bld_i2c_gpio_r;

static int bld_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int bld_i2c_remove(struct i2c_client *client);

static struct i2c_client * bld_i2c_client = NULL;

static const struct i2c_device_id bld_i2c_id[] = {{"breathled",0},{}};   
static struct i2c_board_info __initdata bld_i2c ={ I2C_BOARD_INFO("breathled", (0xb6>>1))};

static int led_ear_sta = 1;
static int led_chest_sta = 1;
static char led_ear_color = 'G';
static char led_chest_color = 'B';
static char led_ear_freq = 'M';
static char led_chest_freq = 'M';

#define DEV_NAME   "breathleds"
static struct dev_t *b_dev = NULL;
static struct cdev *breathleds_cdev;
static struct class *breathleds_class = NULL;
struct device *breathleds_dev = NULL;

static u16 step_base = 0;
static u16 step_count_c = 0;
static u16 step_count_e = 0;

static kal_bool fbl_init_sta = KAL_FALSE;
static kal_bool rbl_init_sta = KAL_FALSE;
static kal_bool lbl_init_sta = KAL_FALSE;


I2C_REGDATA_T init_data_l[] = { 
							{0x12,0x00},
							{0x13,0x00},
/*
							{0x20,0x3f},
							{0x21,0x3f},
							{0x22,0x3f},
							{0x23,0x3f},
							{0x24,0x3f},
							{0x25,0x3f},
							{0x26,0x3f},
							{0x27,0x3f},
							{0x28,0x3f},
							{0x29,0x3f},
							{0x2a,0x3f},
							{0x2b,0x3f},
							{0x2c,0x3f},
							{0x2d,0x3f},
							{0x2e,0x3f},
							{0x2f,0x3f}
*/							
							};
I2C_REGDATA_T init_data_r[] = { 
							{0x12,0x00},
							{0x13,0x00},
/*
							{0x20,0},
							{0x21,0},
							{0x22,0},
							{0x23,0},
							{0x24,0},
							{0x25,0},
							{0x26,0},
							{0x27,0},
							{0x28,0},
							{0x29,0},
							{0x2a,0},
							{0x2b,0},
							{0x2c,0},
							{0x2d,0},
							{0x2e,0},
							{0x2f,0}
*/							
							};

static void breathled_set_color(void)
{
	
}

static void led_set_step1(struct i2c_client *client,BLD_CLR_REG_MAP_T led, u8 step)
{
	if(led.led_sta == KAL_FALSE)
	{
		wrapper_i2c_write_regdata(client, led.reg, 0);
	}else
	{
		wrapper_i2c_write_regdata(client, led.reg, step);		
	}
}

static void led_set_step2(I2C_GPIO_T *dev,BLD_CLR_REG_MAP_T led, u8 step)
{
	if(led.led_sta == KAL_FALSE)
	{
		i2c_write_regdata(dev, led.reg, 0);
	}else
	{
		i2c_write_regdata(dev, led.reg, step);
	}
}

static void breathled_set_step1(struct i2c_client *client, CLR3_LED_T *leds, u8 lednum, U8 *breath_step, u8 steps_index)
{
	int i = 0, j = 0;
	
	for(i = 0; i < lednum; i++)
	{	
		//for(j = 0; j < CLR_MAX; i++)
		{
			led_set_step1(client,leds[i].red, breath_step[steps_index]);
			led_set_step1(client,leds[i].green, breath_step[steps_index]);
			led_set_step1(client,leds[i].blue, breath_step[steps_index]);			
		}
		
	}
}
static void breathled_set_step2(I2C_GPIO_T *dev, CLR3_LED_T *leds, u8 lednum, U8 *breath_step, u8 steps_index)
{
	int i = 0, j = 0;
	
	for(i = 0; i < lednum; i++)
	{	
		//for(j = 0; j < CLR_MAX; i++)
		{
			led_set_step2(dev,leds[i].red, breath_step[steps_index]);
			led_set_step2(dev,leds[i].green, breath_step[steps_index]);
			led_set_step2(dev,leds[i].blue, breath_step[steps_index]);
		}
		
	}
}

static int breathled_color_change(CLR3_LED_T *led, LED_CLR_E color, int length)
{
	int i = 0, j = 0;

	if(led == NULL)
	{
		BLD_LOG("NO leds to set color !\n");
		return -1;
	}	
	for(; i < length; i++)
	{

		if(color & RED)
			led[i].red.led_sta = KAL_TRUE;
		else	
			led[i].red.led_sta = KAL_FALSE;
		
		if(color & GREEN)
			led[i].green.led_sta = KAL_TRUE;
		else	
			led[i].green.led_sta = KAL_FALSE;
		
		if(color & BLUE)
			led[i].blue.led_sta = KAL_TRUE;
		else	
			led[i].blue.led_sta = KAL_FALSE;

	}
	
	return 0;
}

static void breathled_effect(void)
{
	int i = 0, j = 0;

	
	if(lbl_init_sta)
	{
		if((led_ear_sta == 0) && (step_count_e == (BREATH_MAX_STEP - 1)))
			lbl_init_sta = KAL_FALSE;
		breathled_set_step2(&bld_i2c_gpio_l, LBL, sizeof(LBL)/sizeof(CLR3_LED_T), breath_steps[rlf_step[1]], step_count_e);

	}
	if(rbl_init_sta)
	{
		if((led_ear_sta == 0) && (step_count_e == (BREATH_MAX_STEP - 1)))
			rbl_init_sta = KAL_FALSE;
		breathled_set_step2(&bld_i2c_gpio_r, RBL, sizeof(RBL)/sizeof(CLR3_LED_T), breath_steps[rlf_step[2]], step_count_e);	
	}

	if(fbl_init_sta)
	{
		if((led_chest_sta == 0) && (step_count_c == (BREATH_MAX_STEP - 1)))
			fbl_init_sta = KAL_FALSE;
		breathled_set_step1(bld_i2c_client, FBL, sizeof(FBL)/sizeof(CLR3_LED_T), breath_steps[rlf_step[0]], step_count_c);
	}

}

static void breathled_kthrd(void * argc)
{
	//allow_signal(SIGKILL);
	struct sched_param param = { .sched_priority = RTPM_PRIO_AED };//RTPM_PRIO_AED	//RTPM_PRIO_TPD
	sched_setscheduler(current, SCHED_FIFO, &param);
	while(1)
	{
		set_current_state(TASK_INTERRUPTIBLE);
		//每隔1s执行1次，然后睡眠，可被信号提前唤醒
		schedule_timeout_interruptible(0.03*HZ);
		//如果收到信号（SIGKILL），就退出
		if(signal_pending(current))
			break;
		//wait_event(bld_thread_wq, (bld_thread_timeout == KAL_TRUE));
		mutex_lock(&bld_access);
		bld_thread_timeout = KAL_FALSE;
		
		if(rlf_step[0] == STP_LOW)
		{
			step_count_c += step_base;
		}
		else if(rlf_step[0] == STP_MID)
		{
			step_count_c++;
		}
		else if(rlf_step[0] == STP_HIGH)
		{
			step_count_c += 2;
		}

		if(rlf_step[1] == STP_LOW)
		{
			step_count_e += step_base;
		}
		else if(rlf_step[1] == STP_MID)
		{
			step_count_e++;
		}
		else if(rlf_step[1] == STP_HIGH)
		{
			step_count_e += 2;
		}
		step_base++;
		step_base = step_base%2;
		step_count_c = step_count_c % BREATH_MAX_STEP;
		step_count_e = step_count_e % BREATH_MAX_STEP;
		
		breathled_effect();
		mutex_unlock(&bld_access);

		//ktime = ktime_set(BLD_TASK_PERIOD_S, BLD_TASK_PERIOD_NS);
		//hrtimer_forward_now(&bld_kthread_timer, ktime);
		//hrtimer_start(&bld_kthread_timer, ktime, HRTIMER_MODE_REL);
	}
	
	set_current_state(TASK_RUNNING);
}

enum hrtimer_restart bld_kthread_hrtimer_func(struct hrtimer *timer)
{
	BLD_LOG("wake_up bld_thread_wq\n");
	bld_thread_timeout = KAL_TRUE;
	wake_up(&bld_thread_wq);

	return HRTIMER_NORESTART;
}

static void bld_kthread_hrtimer_init(void)
{
	ktime = ktime_set(BLD_TASK_PERIOD_S, BLD_TASK_PERIOD_NS);
	hrtimer_init(&bld_kthread_timer, CLOCK_REALTIME_ALARM, HRTIMER_MODE_REL);
	bld_kthread_timer.function = bld_kthread_hrtimer_func;
	hrtimer_start(&bld_kthread_timer, ktime, HRTIMER_MODE_REL);
}


struct i2c_driver bld_i2c_driver = {  
  	.driver = {
		.owner	= THIS_MODULE,
	 	.name 	= BLD_NAME,
  	},
  	.probe 	= bld_i2c_probe,
  	.remove 	= bld_i2c_remove,
  	.id_table 	= bld_i2c_id,                        
};

static int bld_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	BLD_LOG("bld_i2c_probe\n");

	bld_i2c_client = client;

	fbl_init_sta = wrapper_i2c_write_regs(client, init_data_l, sizeof(init_data_l)/sizeof(I2C_REGDATA_T));
	if(lbl_init_sta == KAL_FALSE && rbl_init_sta == KAL_FALSE && fbl_init_sta == KAL_FALSE)
	{
		
	}
	else
	{	
		thread = kthread_run(breathled_kthrd, 0,BLD_NAME);
		//bld_kthread_hrtimer_init();
	}

	return 0;
}

static int bld_i2c_remove(struct i2c_client *client)
{

	BLD_LOG("bld_i2c_probe\n");

	return 0;
	
}
void bld_i2c_gpio_init()
{
	i2c_init_gpio(&bld_i2c_gpio_l);
	i2c_init_gpio(&bld_i2c_gpio_r);
}

void bld_powen(void)
{
	mt_set_gpio_mode(BLD_VLED_3V3_EN, GPIO_MODE_00);
	mt_set_gpio_dir(BLD_VLED_3V3_EN, GPIO_DIR_OUT);

	mt_set_gpio_out(BLD_VLED_3V3_EN, GPIO_OUT_ONE);

}
void bld_reset(void)
{
	mt_set_gpio_mode(BLD1_RSTN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(BLD1_RSTN_PIN, GPIO_DIR_OUT);

	mt_set_gpio_out(BLD1_RSTN_PIN, GPIO_OUT_ZERO);
	delay_nop_1us(50);
	mt_set_gpio_out(BLD1_RSTN_PIN, GPIO_OUT_ONE);
	delay_nop_1us(20);
	
	mt_set_gpio_mode(BLD2_RSTN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(BLD2_RSTN_PIN, GPIO_DIR_OUT);

	mt_set_gpio_out(BLD2_RSTN_PIN, GPIO_OUT_ZERO);
	delay_nop_1us(50);
	mt_set_gpio_out(BLD2_RSTN_PIN, GPIO_OUT_ONE);
	delay_nop_1us(20);	
	
}

#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_EARLYSUSPEND

static void bld_suspend( struct early_suspend *h )
{
	mutex_lock(&bld_access);
	lbl_init_sta =	KAL_FALSE;
	rbl_init_sta =	KAL_FALSE;
	breathled_set_step2(&bld_i2c_gpio_l, LBL, sizeof(LBL)/sizeof(CLR3_LED_T), breath_steps[rlf_step[1]], BREATH_MAX_STEP - 1);
	breathled_set_step2(&bld_i2c_gpio_r, RBL, sizeof(RBL)/sizeof(CLR3_LED_T), breath_steps[rlf_step[2]], BREATH_MAX_STEP - 1);	
	BLD_LOG("suspend\n");

	mutex_unlock(&bld_access);

}

static void bld_resume( struct early_suspend *h )
{
	mutex_lock(&bld_access);
	lbl_init_sta =	KAL_TRUE;
	rbl_init_sta =	KAL_TRUE;
	BLD_LOG("resume\n");

	mutex_unlock(&bld_access);

}

static struct early_suspend bld_early_suspend_handler = {
	.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 1,
	.suspend = bld_suspend,
	.resume = bld_resume,
};

#endif
#endif

/***************************breathleds***************************
							added by daviekuo
/****************************************************************/
static ssize_t store_frequency(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int reg_value = 0;
	unsigned int reg_address = 0;
	unsigned int index = 0, err = 0;
	printk("daviekuo, store_frequenct in data %s\n",buf);

	if (buf != NULL && size != 0)
	{		
		char led = *(buf+0);
		char freq = *(buf+1);
		
		if(led == 'C' || led == 'c')
		{
			index = 0;
			led_chest_freq = freq;
		}	
		else if(led == 'E' || led == 'e')
		{
			index = 1;
			led_ear_freq = freq;
		}	
		switch(freq)
		{
			case 'L':
			case 'l':						//Low speed
				if(led == 'C' || led == 'c')
					rlf_step[index]	= STP_LOW;
				else if(led == 'E' || led == 'e')
				{
					rlf_step[index] = STP_LOW;
					rlf_step[index+1] = STP_LOW;
				}
				break;		

			case 'M':
			case 'm':						//Middle speed
				if(led == 'C' || led == 'c')
					rlf_step[index]	= STP_MID;
				else if(led == 'E' || led == 'e')
				{
					rlf_step[index] = STP_MID;
					rlf_step[index+1] = STP_MID;
				}

				break;		

			case 'H':
			case 'h':						//High speed
				if(led == 'C' || led == 'c')
					rlf_step[index]	= STP_HIGH;
				else if(led == 'E' || led == 'e')
				{
					rlf_step[index] = STP_HIGH;
					rlf_step[index+1] = STP_HIGH;
				}
				break;
				
			case 'E':
			case 'e':						//Extre speed
				break;
			default:
				return -2;					
		}
	}
	else
		return -1;
	return sprintf(buf, "E%cC%c\n", led_ear_freq, led_chest_freq);
	return 0;

}

static ssize_t show_frequency(struct device *dev, struct device_attribute *attr, char *buf)
{
	char data[20] = {0};
	data[0] = 'E';
	data[1] = led_ear_freq;
	data[2] = 'C';
	data[3] = led_chest_freq;

/*	
	if(led_ear_color == 0)
		data[1] = '0';
	else
		data[1] = '1';
	if(led_chest_sta == 0)
		data[3] = '0';
	else
		data[3] = '1';
*/
	data[4] = '\0';

	return sprintf(buf, "%s\n", data);

	return 0;
}

static DEVICE_ATTR(frequency, 0664, show_frequency, store_frequency);

static ssize_t store_color(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int reg_value = 0;
	unsigned int reg_address = 0;
	unsigned int len = 0, err = 0;

	BLD_LOG("store_color in data %s\n",buf);
	


	if (buf != NULL && size != 0) 
	{		
		char led = *(buf+0);
		char clr = *(buf+1);
		
		CLR3_LED_T *led1 = NULL;
		CLR3_LED_T *led2 = NULL;
		if(led == 'C' || led == 'c')
		{
			led1 = FBL;
			led2 = NULL;
			len = sizeof(FBL)/sizeof(CLR3_LED_T);
			led_chest_color = clr;

		}	
		else if(led == 'E' || led == 'e')
		{

			led1 = RBL;
			led2 = LBL;
			len = sizeof(RBL)/sizeof(CLR3_LED_T);
			led_ear_color = clr;

		}	
		switch(clr)
		{

			case 'R':
			case 'r':
				breathled_color_change(led1, RED, len);
				breathled_color_change(led2, RED, len);

				printk("daviekuo: RRRRRRRR\n");
	 			break;
			case 'G':
			case 'g':
				breathled_color_change(led1, GREEN, len);
				breathled_color_change(led2, GREEN, len);

				printk("daviekuo: GGGGGGGG\n");
				break;
			case 'B':
			case 'b':
				breathled_color_change(led1, BLUE, len);
				breathled_color_change(led2, BLUE, len);
				printk("daviekuo: BBBBBBBB\n");
				break;		
				
			case 'X':						//RG
			case 'x':
				breathled_color_change(led1, RG, len);
				breathled_color_change(led2, RG, len);

				printk("daviekuo: RGRGRGRGRG\n");
				break;
			case 'Y':						//RB
			case 'y':
				breathled_color_change(led1, RB, len);
				breathled_color_change(led2, RB, len);

				printk("daviekuo: RBRBRBRBRB\n");
				
				break;
			case 'Z':
			case 'z':						//BG
				breathled_color_change(led1, BG, len);
				breathled_color_change(led2, BG, len);
				printk("daviekuo: BGBGBGBGBGBG\n");
				break;		
			case 'A':
			case 'a':						//RGB
				breathled_color_change(led1, RGB, len);
				breathled_color_change(led2, RGB, len);
				printk("daviekuo: RGBRGBRGBRGB\n");
				break;
			default:
				return -2;					
		}

	}
	else
		return -1;
	return sprintf(buf, "E%cC%c\n", led_ear_color, led_chest_color);
	return 0;
}

static ssize_t show_color(struct device *dev, struct device_attribute *attr, char *buf)
{
	char data[20] = {0};
	data[0] = 'E';
	data[1] = led_ear_color;
	data[2] = 'C';
	data[3] = led_chest_color;

/*	
	if(led_ear_color == 0)
		data[1] = '0';
	else
		data[1] = '1';
	if(led_chest_sta == 0)
		data[3] = '0';
	else
		data[3] = '1';
*/
	data[4] = '\0';

	return sprintf(buf, "%s\n", data);
}

static DEVICE_ATTR(color, 0664, show_color, store_color);

static ssize_t store_onoff(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
		int reg_data = 0;
		char data[20] = {0};
	
		printk("daviekuo, store_onoff in data %s\n",buf);
		//copy_from_user(data, from, len);
		strcpy(data, buf);
		printk("daviekuo, store_onoff in data %s\n",data);
		char led = *(data+0);
		char sta = *(data+1);
	
		if (size != 0) {
		if(led == 'E' || led == 'e')
		{
			switch(sta)
			{
				case '1':
					rbl_init_sta =	KAL_TRUE;
					lbl_init_sta =	KAL_TRUE;
					led_ear_sta = 1;

					break;
				case '0':
					
					led_ear_sta = 0;
					break;
				default:
					return -2;					
			}
		}
		else if(led == 'C' || led == 'c')
		{
			switch(sta)
			{
				case '1':	
				fbl_init_sta = KAL_TRUE;
				led_chest_sta = 1;
					break;
				case '0':

					led_chest_sta = 0;
					break;

				default:
					return -2;					
			}
	
		}
		else if(led == 'A' || led == 'a')
		{
			
		}
	
		}
		else
			return -1;
		return sprintf(buf, "E%dC%d\n", led_ear_sta, led_chest_sta);
		return 0;

}

static ssize_t show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("daviekuo, show_onoff in data\n");

	char data[20] = {0};
	data[0] = 'E';
	data[2] = 'C';
	
	if(led_ear_sta == 0)
		data[1] = '0';
	else
		data[1] = '1';
	if(led_chest_sta == 0)
		data[3] = '0';
	else
		data[3] = '1';
	data[4] = '\0';
	//copy_to_user(to, data, 4);
	return sprintf(buf, "%s\n", data);
}

static DEVICE_ATTR(onoff, 0664, show_onoff, store_onoff);


static long breathleds_unlocked_ioctl(struct file *pfile, unsigned int cmd, unsigned long param)
{
	 return 0;	
}


static int breathleds_release (struct inode *node, struct file *file)
{
 	return 0;
}

static int breathleds_open (struct inode *inode, struct file *file)
{
	printk(KERN_INFO"/dev/breathleds open success!\n");
	return 0;
}

static int breathleds_write(struct file *pfile, const char __user *from, size_t len, loff_t * offset)
{
	int reg_data = 0;
	char data[20] = {0};


	copy_from_user(data, from, len);

	char led = *(data+0);
	char sta = *(data+1);
	printk("daviekuo, breathleds_write  %s\n",data);

	if (len != 0) {
	if(led == 'E' || led == 'e')
	{

		switch(sta)
		{
			case '1':
				rbl_init_sta = 	KAL_TRUE;
				lbl_init_sta = 	KAL_TRUE;
				led_ear_sta = 1;
	 			break;
			case '0':


				led_ear_sta = 0;

				break;
			default:
				return -2;					
		}
	}
	else if(led == 'C' || led == 'c')
	{

		switch(sta)
		{
			case '1':						
				fbl_init_sta = KAL_TRUE;
				led_chest_sta = 1;
	 			break;
			case '0':
    
				led_chest_sta = 0;
				break;			
			default:
				return -2;					
		}

	}
	else if(led == 'A' || led == 'a')
	{
		
	}

	}
	else
		return -1;
	
	return 1;
}

static int breathleds_read (struct file *pfile, char __user *to, size_t len, loff_t *offset)
{	
	char data[20] = {0};
	data[0] = 'E';
	data[2] = 'C';
	
	if(led_ear_sta == 0)
		data[1] = '0';
	else
		data[1] = '1';
	if(led_chest_sta == 0)
		data[3] = '0';
	else
		data[3] = '1';

	copy_to_user(to, data, 4);
	
	return 0;

}

static struct file_operations breathleds_fops = {
	.owner = THIS_MODULE,
	.open = breathleds_open,
	.write = breathleds_write,
	.read = breathleds_read,
	.release = breathleds_release,
	.unlocked_ioctl = breathleds_unlocked_ioctl,
};

static int breathleds_probe(struct platform_device *pdev)
{
	int ret;
	
	BLD_LOG("breathleds  init\n");
	bld_powen();
	bld_reset();
	bld_i2c_gpio_init();
	
	bld_i2c_gpio_l.scl = BLD_I2C_SCL_L_PIN;
	bld_i2c_gpio_l.sda = BLD_I2C_SDA_L_PIN;
	bld_i2c_gpio_l.addr = BLD_I2C_ADDR_L;
	
	lbl_init_sta = i2c_write_regs(&bld_i2c_gpio_l, init_data_l, sizeof(init_data_l)/sizeof(I2C_REGDATA_T));
	
	bld_i2c_gpio_r.scl = BLD_I2C_SCL_R_PIN;
	bld_i2c_gpio_r.sda = BLD_I2C_SDA_R_PIN;
	bld_i2c_gpio_r.addr = BLD_I2C_ADDR_R;
	rbl_init_sta = i2c_write_regs(&bld_i2c_gpio_r, init_data_r, sizeof(init_data_r)/sizeof(I2C_REGDATA_T));

	i2c_register_board_info(BLD_I2C_BUSNUM, &bld_i2c, 1);

	if(i2c_add_driver(&bld_i2c_driver) < 0)
	{
		BLD_ERR("add driver failed\n");
	 	return -1;
	}

	ret = alloc_chrdev_region(&b_dev, 0, 1, DEV_NAME);
		 if (ret< 0) {
		 printk("breathleds	alloc_chrdev_region failed, %d", ret);
		return ret;
	}
	breathleds_cdev= cdev_alloc();
	if (breathleds_cdev == NULL) {
			 printk("breathleds cdev_alloc failed");
			 ret = -ENOMEM;
			 goto EXIT;
	}
	cdev_init(breathleds_cdev, &breathleds_fops);
	 breathleds_cdev->owner = THIS_MODULE;
	 ret = cdev_add(breathleds_cdev, b_dev, 1);
	 if (ret < 0) {
		  printk("Attatch file breathleds operation failed, %d", ret);
		 goto EXIT;
	 }
	
	
	  breathleds_class = class_create(THIS_MODULE, "yyd");
	  if (IS_ERR(breathleds_class)) {
		  printk("Failed to create class(breathleds)!\n");
		  return PTR_ERR(breathleds_class);
	  }
			  
	  breathleds_dev = device_create(breathleds_class, NULL, b_dev, NULL,DEV_NAME);
	  if (IS_ERR(breathleds_dev))
	  	
		  printk("Failed to create breathleds device\n");
	  if (device_create_file(breathleds_dev, &dev_attr_onoff) < 0)
			  BLD_ERR("Failed to create device file(%s)!\n",
					dev_attr_onoff.attr.name);	 
	  if (device_create_file(breathleds_dev, &dev_attr_color) < 0)
			  BLD_ERR("Failed to create device file(%s)!\n",
					dev_attr_color.attr.name);
	  if (device_create_file(breathleds_dev, &dev_attr_frequency) < 0)
			  BLD_ERR("Failed to create device file(%s)!\n",
					dev_attr_frequency.attr.name);


	BLD_LOG("breathleds  finish\n");
	
	return ret;

EXIT:

	if(breathleds_cdev != NULL)
	{
		cdev_del(breathleds_cdev);
		breathleds_cdev = NULL;
	}

	 unregister_chrdev_region(breathleds_dev, 1);

	return ret;

}


static int breathleds_remove(struct platform_device *pdev)
{
    return 0;
}

static int breathleds_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	//mutex_lock(&bld_access);
	lbl_init_sta =	KAL_FALSE;
	rbl_init_sta =	KAL_FALSE;
	breathled_set_step2(&bld_i2c_gpio_l, LBL, sizeof(LBL)/sizeof(CLR3_LED_T), breath_steps[rlf_step[1]], BREATH_MAX_STEP - 1);
	breathled_set_step2(&bld_i2c_gpio_r, RBL, sizeof(RBL)/sizeof(CLR3_LED_T), breath_steps[rlf_step[2]], BREATH_MAX_STEP - 1);	
	BLD_LOG("suspend\n");

	//mutex_unlock(&bld_access);

    return 0;
}
static int breathleds_shutdown(struct platform_device *pdev)
{
	return 0;
}

static int breathleds_resume(struct platform_device *pdev)
{
	mutex_lock(&bld_access);
	lbl_init_sta =	KAL_TRUE;
	rbl_init_sta =	KAL_TRUE;
	BLD_LOG("resume\n");

	mutex_unlock(&bld_access);

    return 0;
}


static struct platform_device breathleds_device = {
	.name = "breathleds",
	.id = -1
};


// platform structure
static struct platform_driver breathleds_driver = {
    .probe		= breathleds_probe,
    .remove	= breathleds_remove,
    .shutdown = breathleds_shutdown,
    .suspend	= breathleds_suspend,
    .resume	= breathleds_resume,
    .driver		= {
        .name	= "breathleds",
        .owner	= THIS_MODULE,
    }
};

static int __init breathleds_init(void)
{
	int ret;

	ret = platform_device_register(&breathleds_device);
	if (ret)
		BLD_ERR("breathleds_device_init: %d\n", ret);
	
	ret = platform_driver_register(&breathleds_driver);

	if (ret) {
		BLD_ERR("breathleds_init: %d\n", ret);
		return ret;
	}
	#ifdef CONFIG_HAS_EARLYSUSPEND
	#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&bld_early_suspend_handler);
	#endif
	#endif
	
	return ret;
}

static void __exit breathleds_exit(void)
{
	platform_driver_unregister(&breathleds_driver);
}

module_init(breathleds_init);
module_exit(breathleds_exit);

MODULE_AUTHOR("MediaTek Inc.");
MODULE_DESCRIPTION("BreathLed driver for MediaTek");
MODULE_LICENSE("GPL");

