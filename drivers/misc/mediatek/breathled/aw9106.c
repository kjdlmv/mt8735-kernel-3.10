
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

#include <linux/i2c.h>
#include <mach/mt_gpio.h>
#include <linux/earlysuspend.h>

#define BREATHLEDS_I2C_BUSNUM 3

static int breathleds_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int breathleds_i2c_remove(struct i2c_client *client);

static struct i2c_client * breathleds_i2c_client = NULL;

static int led_ear_sta = 1;
static int led_chest_sta = 1;
static char led_ear_color = 'O';
static char led_chest_color = 'O';
static char led_ear_freq = 'L';
static char led_chest_freq = 'L';


static const struct i2c_device_id breathleds_i2c_id[] = {{"breathled_ear",0},{"breathled_chest",0},{}};   
static struct i2c_board_info __initdata breathleds_i2c_hw1={ I2C_BOARD_INFO("breathled_chest", (0xb2>>1))};
static struct i2c_board_info __initdata breathleds_i2c_hw2={ I2C_BOARD_INFO("breathled_ear", (0xb6>>1))};


struct i2c_driver breathleds_i2c_driver = {                       
    .probe = breathleds_i2c_probe,                                   
    .remove = breathleds_i2c_remove,                           
    .driver= 
    {
	.owner	= THIS_MODULE,
	.name	= breathleds_i2c_id,
     },                 
    .id_table = breathleds_i2c_id,                             
};


#define AW9106_RESET_PIN		    GPIO124                 //AW9106的RESET脚
#define IIC_ADDRESS_WRITE			0xB0        //I
#define IIC_ADDRESS_READ			0xB1        //IIC的读地址，={1011，0，AD1，AD0，1}，AD0,AD1接低则为0xB1 ，接高则是0xB7
#define AW9016_I2C_MAX_LOOP 		5


/****************************************************************************
 * DEBUG MACROS
 ***************************************************************************/
static int debug_enable_led = 1;
#define LEDS_DRV_DEBUG(format, args...) do { \
	if (debug_enable_led) \
	{\
		printk(KERN_WARNING format, ##args);\
	} \
} while (0)


void AW9106_delay_1us(U16 wTime)   //延时1us的函尸
{
// 注意，根据各平台的主频调整为1us .
    U16 i, j;
#if defined(MT6223) 
    for (i=0; i<wTime*30; i++) ;  
#elif defined(MT6235)
    for (i=0; i<wTime*8; i++) ;
#else
    for (i=0; i<wTime*15; i++) ;
#endif
}


static BOOL AW9016_i2c_write_reg_org(unsigned char reg,unsigned char data)
{
	BOOL ack=0;
	unsigned char ret;
	unsigned char wrbuf[2];

	wrbuf[0] = reg;
	wrbuf[1] = data;

	ret = i2c_master_send(breathleds_i2c_client, wrbuf, 2);
	if (ret != 2) {
		printk(&breathleds_i2c_client->dev,
		"%s: i2c_master_recv() failed, ret=%d\n",
		__func__, ret);
		ack = 1;
	}

	return ack;
}

BOOL AW9016_i2c_write_reg(unsigned char reg,unsigned char data)
{
	BOOL ack=0;
	unsigned char i;
	for (i=0; i<AW9016_I2C_MAX_LOOP; i++)
	{
		ack = AW9016_i2c_write_reg_org(reg,data);
		if (ack == 0) // ack success
			break;
		}
	return ack;
}

int AW9016_WriteRegs(u16 addr, char * buf ,int count)
{
	unsigned char ret;
	 printk("************   8888iddr=0x%02x  \n",breathleds_i2c_client->addr);
	ret = i2c_master_send(breathleds_i2c_client, buf, count);
	
	if (ret != count) 
	{

		   printk("**********************   8888   ning s4aw2013_WriteReg failed  %s \r\n", __func__);

	
		dev_err(&breathleds_i2c_client->dev,"%s: i2c_master_recv() failed, ret=%d\n",
			__func__, ret);
	}
	 printk("*********5555***   8888  \n");
	return ret;
}

unsigned char AW9016_i2c_read_reg(unsigned char regaddr) 
{
	unsigned char rdbuf[1], wrbuf[1], ret, i;

	wrbuf[0] = regaddr;

	for (i=0; i<AW9016_I2C_MAX_LOOP; i++) 
	{
		ret = i2c_master_send(breathleds_i2c_client, wrbuf, 1);
		if (ret == 1)
			break;
	}
	
	ret = i2c_master_recv(breathleds_i2c_client, rdbuf, 1);
	
	if (ret != 1)
	{
		   printk("**********************   5555   ning AW2013_i2c_read_reg failed  %s \r\n", __func__);
	
		dev_err(&breathleds_i2c_client->dev,"%s: i2c_master_recv() failed, ret=%d\n",
			__func__, ret);
	}
	
    	return rdbuf[0];
		
}

static BOOL AW9106_i2c_write_reg(unsigned char reg,unsigned char data)
{
	return AW9016_i2c_write_reg(reg, data);

}

U8 AW9106_i2c_read_reg(U8 regaddr) 
{
	return AW9016_i2c_read_reg(regaddr);

}

void AW9106_Hw_reset(void)
{   
	mt_set_gpio_mode(AW9106_RESET_PIN | 0x80000000, GPIO_MODE_00);
	mt_set_gpio_dir(AW9106_RESET_PIN | 0x80000000,GPIO_DIR_OUT);
	mt_set_gpio_out(AW9106_RESET_PIN | 0x80000000,0);
	AW9106_delay_1us(1000); //复位信号为低电平的持续时间必须至少20us才能正常复位  
	mt_set_gpio_out(AW9106_RESET_PIN | 0x80000000,1);
	AW9106_delay_1us(300); 
}

void AW9106_SoftReset()
{		
	AW9106_i2c_write_reg(0x7f,0x00); //软复位，清除所有寄存器值，灭掉所有灯
	AW9106_delay_1us(30); 
}

void AW9106_POWER_ON(void)
{    // AW9106 POWER-ON， 请客户不要改动此函数
     // 在AW9106_init()中，先进行POWER-ON，再进行客户自身的相关操作
	kal_uint16 count=0;
	//AW9106_i2c_initial();
	AW9106_Hw_reset();
	
}


/*
void AW9106_OnOff(BOOL OnOff)  //AW9106硬件使能。低电平复位，高电平正常工作。
{   
	//uem_stop_timer(AW9106_timer_ID);   //关闭timer
	//GPTI_StopItem(AW9106_timer_ID);
	//StopTimer(AW9106_timer_ID);
	 Paoma_cnt=0;
   	Breath_cnt=0;
	GPIO_ModeSetup(AW9106_RESET_PIN, 0);
	GPIO_InitIO(1, AW9106_RESET_PIN);     
	AW9106_i2c_initial();  
    AW9106_i2c_write_reg(0x7f,0x00); //软复位，可以不要	  
	GPIO_WriteIO(0, AW9106_RESET_PIN);
	AW9106_delay_1us(200); //复位信号为低电平的持续时间必须至少20us才能正常复位	
	if (OnOff ==1)
	{  
		GPIO_WriteIO(1, AW9106_RESET_PIN); 
	}
	AW9106_delay_1us(30); 
}
*/

//-------------------------------------------------------------------------------------------
//函数名: AW9106_AllOn
//调用此函数，则10路灯全亮，每路的电流分别由软件独立控制。
//-------------------------------------------------------------------------------------------
void AW9106_AllOn(void)
{
	printk("-------------------------AW9106_AllOn  Entry ------------------------- \r\n");

	AW9106_SoftReset();
	AW9106_i2c_write_reg(0x12,0x00);   //OUT配置为呼吸灯模式
	AW9106_i2c_write_reg(0x13,0x00);   //OUT配置为呼吸灯模式
	
	AW9106_i2c_write_reg(0x20,0x3f);//OUT0口调光，调光等级为0-255。OUT0~OUT5的调光指令依次为0x20~0x25. 写0关闭
	AW9106_i2c_write_reg(0x21,0x3f);
	AW9106_i2c_write_reg(0x22,0x3f);
	AW9106_i2c_write_reg(0x23,0x3f);
	AW9106_i2c_write_reg(0x24,0x3f);
	AW9106_i2c_write_reg(0x25,0x3f);

}

void AW9106_out0_fade(void)
{
	AW9106_i2c_write_reg(0x12,0x00);   //OUT4~5配置为呼吸灯模式这句如果之前设置过，可以不要
	AW9106_i2c_write_reg(0x13,0x00);   //OUT0~3配置为呼吸灯模式这句如果之前设置过，可以不要
	AW9106_i2c_write_reg(0x04,0x03); 	 //OUT4-OUT5自主呼吸BLINK模式使模?0为blink模式，1为FADE模式
	AW9106_i2c_write_reg(0x05,0x0e);   //OUT0-OUT3自主呼吸BLINK模式使能 0为blink模式，1为FADE模式?	AW9106_i2c_write_reg(0x15,0x09);   //淡进淡出时间设置这句如果之前设置过，可以不要

	AW9106_i2c_write_reg(0x03,0x00);   //先把03H清0
	AW9106_i2c_write_reg(0x03,0x01);   //从0写到1则产生淡进过程
	//AW9106_i2c_write_reg(0x03,0x00);   //从1写到0则产生淡出? 
}

//-------------------------------------------------------------------------------------------
//以下为AW9106实现几种效果的参考函数，用户请根据自己的效果需要来调整。
//-------------------------------------------------------------------------------------------

void AW9106_init(void)    //AW9106初始化，请客户在开机初始化时调用
{
	//AW9106_POWER_ON();   //AW9106 POWER-ON ，客户一般不要改动
	//先POWER-ON，以下再进行客户需要的操作
	//以下为客户初始化，由客户更改
	//AW9106_i2c_initial();
	//AW9106_Hw_reset();
	AW9106_SoftReset();	//GPTI_GetHandle(&AW9106_timer_ID);  ????daviekuo
	AW9106_i2c_write_reg(0x12,0x00);   //P0口配置为呼吸灯模式
	AW9106_i2c_write_reg(0x13,0x00);   //P1口配置为呼吸灯模式
	
	AW9106_i2c_write_reg(0x20,0x3f);//OUT0口调光，调光等级为0-255。OUT0~OUT5的调光指令依次为0x20~0x2f. 写0关闭
	AW9106_i2c_write_reg(0x21,0x3f);
	AW9106_i2c_write_reg(0x22,0x3f);
	AW9106_i2c_write_reg(0x23,0x3f);
	AW9106_i2c_write_reg(0x24,0x3f);
	AW9106_i2c_write_reg(0x25,0x3f);		
	//AW9106_test();  //读取AW9106内部寄存器的值，看与写进去的值是否一致，由此判断I2C接口是否通畅
}

//-------------------------------------------------------------------------------------------
//函数名: AW9106_init_pattern
//调用此函数，则6路灯全亮，为自主呼吸BLINK 模式
//-------------------------------------------------------------------------------------------
void AW9106_init_pattern(void)
{
 //AW9106基本的效果实现。6路自主呼吸
		printk(KERN_ERR"-------------------------AW9106_init_pattern  Entry ------------------------- \r\n");
	  AW9106_SoftReset();
	 if(1)//(strcmp(breathleds_i2c_client->name, "breathled_chest")==0)
	 {
	 //printk("daviekuo zzzzzzzzzz  0x%x\n", breathleds_i2c_client->addr);

	  AW9106_i2c_write_reg(0x12,0x00);	//OUT4~5配置为呼吸灯模式
	  AW9106_i2c_write_reg(0x13,0x00);	//OUT0~3配置为呼吸灯模式
	  AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能 	 
 
	  AW9106_i2c_write_reg(0x04,0x00);	 //OUT4-OUT5自主呼吸BLINK模式使能		 out5/out2 R  out4/out1 G	out3/out0 B  
	  AW9106_i2c_write_reg(0x05,0x09);	 //OUT0-OUT3自主呼吸BLINK模式使能
 /*   
	  AW9106_i2c_write_reg(0x15,0x1b);	  //淡进淡出时间设置	 (512+1024)  +	(1024+512)
	  AW9106_i2c_write_reg(0x16,0x12);	  //全亮全暗时间设置 

	  AW9106_i2c_write_reg(0x15,0x02);	 //淡进淡出时间设置 	(2048 + 512)	+  (256 + 0)
	  AW9106_i2c_write_reg(0x16,0x20);	 //全亮全暗时间设置 

	  AW9106_i2c_write_reg(0x15,0x1b);	 //淡进淡出时间设置 	(512+1024)	+  (512+1024)
	  AW9106_i2c_write_reg(0x16,0x12);	 //全亮全暗时间设置 
 */ 	  
	  AW9106_i2c_write_reg(0x15,0x12);    //淡进淡出时间设置	  (256+512)  +	 (256+512)
	  AW9106_i2c_write_reg(0x16,0x20);    //全亮全暗时间设置 	  
	 
//	  AW9106_i2c_write_reg(0x11,0x82);	 //开始自主呼吸，并设置最大电流
	  AW9106_i2c_write_reg(0x11,0x00);   //开始自主呼吸，并设置最大电流		//由android laucher来设置初始状态
	  led_ear_freq = 'M';
	  led_chest_freq = 'M';
	 }
	 else
	 {
		 AW9106_SoftReset();
		 AW9106_i2c_write_reg(0x12,0x03);   //OUT4~5配置为呼吸灯模式
		 AW9106_i2c_write_reg(0x13,0x0f);   //OUT0~3配置为呼吸灯模式
	/*	 
		 AW9106_i2c_write_reg(0x04,0x03);   //OUT4-OUT5自主呼吸BLINK模式使能		out all
		 AW9106_i2c_write_reg(0x05,0x0f);   //OUT0-OUT3自主呼吸BLINK模式使能
	*/

		 AW9106_i2c_write_reg(0x04,0x01);   //OUT4-OUT5自主呼吸BLINK模式使能		out5/out2 R  out4/out1 G   out3/out0 B  
		 AW9106_i2c_write_reg(0x05,0x02);   //OUT0-OUT3自主呼吸BLINK模式使能
	/*	 
		 AW9106_i2c_write_reg(0x15,0x1b); 	 //淡进淡出时间设置		(512+1024)  +  (512+1024)
		 AW9106_i2c_write_reg(0x16,0x12);	 //全亮全暗时间设置 
		 AW9106_i2c_write_reg(0x15,0x12);	  //淡进淡出时间设置	 (256+512)  +	(256+512)
		 AW9106_i2c_write_reg(0x16,0x09);	  //全亮全暗时间设置 
	*/

		 AW9106_i2c_write_reg(0x02,0x1);	
		 AW9106_i2c_write_reg(0x03,0x2);	

		 //AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能 

//		 AW9106_i2c_write_reg(0x11,0x82);	//开始自主呼吸，并设置最大电流
		 AW9106_i2c_write_reg(0x11,0x02);	//开始自主呼吸，并设置最大电流	   //由android laucher来设置初始状态
	 }
	 //如果不采用自主呼吸模式，则采用下列方式调光
	//AW9106_i2c_write_reg(0x14,0x00);//关闭自主呼吸使能
	//AW9106_i2c_write_reg(0x20,0x3f);//OUT0口调光，调光等级为0-255。OUT0~OUT5的调光指令依次为0x20~0x25. 写0关闭
	//AW9106_i2c_write_reg(0x21,0x3f);
	//AW9106_i2c_write_reg(0x22,0x3f);
	//AW9106_i2c_write_reg(0x23,0x3f);
	//AW9106_i2c_write_reg(0x24,0x3f);
	//AW9106_i2c_write_reg(0x25,0x3f);
		AW9106_delay_1us(60); 
	 //AW9106_test();  //读取AW9106内部寄存器的值，看与写进去的值是否一致，由此判断I2C接口是否通畅
}

#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_EARLYSUSPEND

static void b_suspend( struct early_suspend *h )
{
	breathleds_i2c_client->addr = 0xb6>>1;

	AW9106_i2c_write_reg(0x012,0x3);	  
	AW9106_i2c_write_reg(0x013,0xf);
	led_ear_sta = 0;
	
	breathleds_i2c_client->addr = 0xb2>>1;

	AW9106_i2c_write_reg(0x012,0x3);	  
	AW9106_i2c_write_reg(0x013,0xf);

	led_chest_sta = 0;

}
 
static void b_resume( struct early_suspend *h )
{
	breathleds_i2c_client->addr = 0xb6>>1;

	AW9106_i2c_write_reg(0x012,0x0);	  
	AW9106_i2c_write_reg(0x013,0x0);							
	AW9106_i2c_write_reg(0x11,0x80);

	led_ear_sta = 1;

	breathleds_i2c_client->addr = 0xb2>>1;

	AW9106_i2c_write_reg(0x012,0x0);	  
	AW9106_i2c_write_reg(0x013,0x0);							
	AW9106_i2c_write_reg(0x11,0x80);

	led_chest_sta = 1;	
}
 
 static struct early_suspend breathleds_early_suspend_handler = {
	 .level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 1,
	 .suspend = b_suspend,
	 .resume = b_resume,
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
	unsigned int err = 0;
	printk("daviekuo, store_frequenct in data %s\n",buf);
	//breathleds_i2c_client->addr = 0xb6>>1;
	if (buf != NULL && size != 0)
	{		
		char led = *(buf+0);
		char freq = *(buf+1);
		
		if(led == 'C' || led == 'c')
		{
			breathleds_i2c_client->addr = 0xb2>>1;
			led_chest_freq = freq;
		}	
		else if(led == 'E' || led == 'e')
		{
			breathleds_i2c_client->addr = 0xb6>>1;
			led_ear_freq = freq;
		}	
		switch(freq)
		{
			case 'L':
			case 'l':						//Low speed
				AW9106_i2c_write_reg(0x15,0x12);   //淡进淡出时间设置	  (256 + 512)	  +  (256 + 512)
				AW9106_i2c_write_reg(0x16,0x09);   //全亮全暗时间设置 
				break;		

			case 'M':
			case 'm':						//Middle speed
				AW9106_i2c_write_reg(0x15,0x12);   //淡进淡出时间设置	  (2048 + 512)	  +  (0 + 512)
				AW9106_i2c_write_reg(0x16,0x20);   //全亮全暗时间设置 
				break;		

			case 'H':
			case 'h':						//High speed
				AW9106_i2c_write_reg(0x15,0x09);   //淡进淡出时间设置	  (256 + 256)	  +  (0 + 256)
				AW9106_i2c_write_reg(0x16,0x08);   //全亮全暗时间设置 
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
	
	return 0;

}

static ssize_t show_frequency(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static DEVICE_ATTR(frequency, 0664, show_frequency, store_frequency);

static ssize_t store_color(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
	char *pvalue = NULL;
	unsigned int reg_value = 0;
	unsigned int reg_address = 0;
	unsigned int err = 0;
	printk("daviekuo, store_color in data %s\n",buf);
	//breathleds_i2c_client->addr = 0xb6>>1;

	if (buf != NULL && size != 0) 
	{		
		char led = *(buf+0);
		char clr = *(buf+1);
		
		if(led == 'C' || led == 'c')
		{
			breathleds_i2c_client->addr = 0xb2>>1;
			led_chest_color = clr;
		}	
		else if(led == 'E' || led == 'e')
		{
			breathleds_i2c_client->addr = 0xb6>>1;
			led_ear_color = clr;
		}	
		switch(clr)
		{

			case 'R':
			case 'r':
				AW9106_SoftReset();				
				AW9106_i2c_write_reg(0x12,0x00);   //OUT4~5配置为呼吸灯模式
				AW9106_i2c_write_reg(0x13,0x00);   //OUT0~3配置为呼吸灯模式
				AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能		
				AW9106_i2c_write_reg(0x04,0x02);   //OUT4-OUT5自主呼吸BLINK模式使能 	out5/out2 R  out4/out1 G   out3/out0 B	
				AW9106_i2c_write_reg(0x05,0x04);   //OUT0-OUT3自主呼吸BLINK模式使能		
					
				AW9106_i2c_write_reg(0x11,0x80);
				printk("daviekuo: RRRRRRRR\n");
	 			break;
			case 'G':
			case 'g':
				 AW9106_SoftReset();				
				 AW9106_i2c_write_reg(0x12,0x00);	//OUT4~5配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x13,0x00);	//OUT0~3配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能
				 AW9106_i2c_write_reg(0x04,0x01);	//OUT4-OUT5自主呼吸BLINK模式使能		out5/out2 R  out4/out1 G   out3/out0 B	
				 AW9106_i2c_write_reg(0x05,0x02);	//OUT0-OUT3自主呼吸BLINK模式使能
				 //AW9106_i2c_write_reg(0x15,0x12);	  //淡进淡出时间设置	 (256+512)	+	(256+512)
				 //AW9106_i2c_write_reg(0x16,0x09);	  //全亮全暗时间设置 
				 AW9106_i2c_write_reg(0x11,0x80);
				printk("daviekuo: GGGGGGGG\n");
				
				break;
			case 'B':
			case 'b':
	//			 AW9106_Hw_reset();
			     AW9106_SoftReset(); 			
				 AW9106_i2c_write_reg(0x12,0x00);	//OUT4~5配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x13,0x00);	//OUT0~3配置为呼吸灯模式
			     AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能		
				 AW9106_i2c_write_reg(0x04,0x00);	//OUT4-OUT5自主呼吸BLINK模式使能		out5/out2 R  out4/out1 G   out3/out0 B	
				 AW9106_i2c_write_reg(0x05,0x09);	//OUT0-OUT3自主呼吸BLINK模式使能
				 //AW9106_i2c_write_reg(0x15,0x12);	 //淡进淡出时间设置    (256+512)  +   (256+512)
				 //AW9106_i2c_write_reg(0x16,0x09);	 //全亮全暗时间设置 			 
				 AW9106_i2c_write_reg(0x11,0x80);				
				break;		
				
			case 'X':						//RG
			case 'x':
				AW9106_SoftReset(); 			
				AW9106_i2c_write_reg(0x12,0x00);   //OUT4~5配置为呼吸灯模式
				AW9106_i2c_write_reg(0x13,0x00);   //OUT0~3配置为呼吸灯模式
				AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能		
				AW9106_i2c_write_reg(0x04,0x03);   //OUT4-OUT5自主呼吸BLINK模式使能 	out5/out2 R  out4/out1 G   out3/out0 B	
				AW9106_i2c_write_reg(0x05,0x06);   //OUT0-OUT3自主呼吸BLINK模式使能 	
				//AW9106_i2c_write_reg(0x15,0x12);	//淡进淡出时间设置	  (256+512)  +	 (256+512)
				//AW9106_i2c_write_reg(0x16,0x09);	//全亮全暗时间设置				
				AW9106_i2c_write_reg(0x11,0x80);
				printk("daviekuo: RRRRRRRR\n");
				break;
			case 'Y':						//BR
			case 'y':
				 AW9106_SoftReset();				
				 AW9106_i2c_write_reg(0x12,0x00);	//OUT4~5配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x13,0x00);	//OUT0~3配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能
				 AW9106_i2c_write_reg(0x04,0x02);	//OUT4-OUT5自主呼吸BLINK模式使能		out5/out2 R  out4/out1 G   out3/out0 B	
				 AW9106_i2c_write_reg(0x05,0x0d);	//OUT0-OUT3自主呼吸BLINK模式使能
				 //AW9106_i2c_write_reg(0x15,0x12);	  //淡进淡出时间设置	 (256+512)	+	(256+512)
				 //AW9106_i2c_write_reg(0x16,0x09);	  //全亮全暗时间设置 
				 AW9106_i2c_write_reg(0x11,0x80);
				printk("daviekuo: GGGGGGGG\n");
				
				break;
			case 'Z':
			case 'z':						//BG
	//			 AW9106_Hw_reset();
				 AW9106_SoftReset();			
				 AW9106_i2c_write_reg(0x12,0x00);	//OUT4~5配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x13,0x00);	//OUT0~3配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能 	
				 AW9106_i2c_write_reg(0x04,0x01);	//OUT4-OUT5自主呼吸BLINK模式使能		out5/out2 R  out4/out1 G   out3/out0 B	
				 AW9106_i2c_write_reg(0x05,0x0b);	//OUT0-OUT3自主呼吸BLINK模式使能
				 //AW9106_i2c_write_reg(0x15,0x12);	 //淡进淡出时间设置    (256+512)  +   (256+512)
				 //AW9106_i2c_write_reg(0x16,0x09);	 //全亮全暗时间设置 			 
				 AW9106_i2c_write_reg(0x11,0x80);				
				break;		
			case 'A':
			case 'a':						//White
	//			 AW9106_Hw_reset();
				 AW9106_SoftReset();			
				 AW9106_i2c_write_reg(0x12,0x00);	//OUT4~5配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x13,0x00);	//OUT0~3配置为呼吸灯模式
				 AW9106_i2c_write_reg(0x14,0x3f);//自主呼吸使能 	
				 AW9106_i2c_write_reg(0x04,0x03);	//OUT4-OUT5自主呼吸BLINK模式使能		out5/out2 R  out4/out1 G   out3/out0 B	
				 AW9106_i2c_write_reg(0x05,0x0f);	//OUT0-OUT3自主呼吸BLINK模式使能
				 //AW9106_i2c_write_reg(0x15,0x12);	 //淡进淡出时间设置    (256+512)  +   (256+512)
				 //AW9106_i2c_write_reg(0x16,0x09);	 //全亮全暗时间设置 			 
				 AW9106_i2c_write_reg(0x11,0x80);				
				break;
			default:
				return -2;					
		}
		if(led == 'E' || led == 'e')
		{
			if(led_ear_freq == 'L' || led_ear_freq == 'l')
			{
				AW9106_i2c_write_reg(0x15,0x12);	  //淡进淡出时间设置	  (256+512)  +	 (256+512)
				AW9106_i2c_write_reg(0x16,0x09);	  //全亮全暗时间设置				
			}
			else if(led_ear_freq == 'M' || led_ear_freq == 'm')
			{
				AW9106_i2c_write_reg(0x15,0x12);   //淡进淡出时间设置	  (2048 + 512)	  +  (256 + 0)
				AW9106_i2c_write_reg(0x16,0x20);   //全亮全暗时间设置 
			}
			else if(led_ear_freq == 'H' || led_ear_freq == 'h')
			{
				AW9106_i2c_write_reg(0x15,0x09);   //淡进淡出时间设置	  (256 + 256)	  +  (0 + 256)
				AW9106_i2c_write_reg(0x16,0x08);   //全亮全暗时间设置 
			}

		}
		else if(led == 'C' || led == 'c')
		{
			if(led_chest_freq == 'L' || led_chest_freq == 'l')
			{
				AW9106_i2c_write_reg(0x15,0x12);	  //淡进淡出时间设置	  (256+512)  +	 (256+512)
				AW9106_i2c_write_reg(0x16,0x09);	  //全亮全暗时间设置				
			}
			else if(led_chest_freq == 'M' || led_chest_freq == 'm')
			{
				AW9106_i2c_write_reg(0x15,0x12);   //淡进淡出时间设置	  (2048 + 512)	  +  (256 + 0)
				AW9106_i2c_write_reg(0x16,0x20);   //全亮全暗时间设置 
			}
			else if(led_chest_freq == 'H' || led_chest_freq == 'h')
			{
				AW9106_i2c_write_reg(0x15,0x09);   //淡进淡出时间设置	  (256 + 256)	  +  (0 + 256)
				AW9106_i2c_write_reg(0x16,0x08);   //全亮全暗时间设置 
			}

		}
	}
	else
		return -1;
	
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
	//copy_to_user(to, data, 4);
	strcpy(buf, data);
	return 0;
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
			breathleds_i2c_client->addr = 0xb6>>1;
			switch(sta)
			{
				case '1':
				/*	
					AW9106_i2c_write_reg(0x02,0x3);    
					AW9106_i2c_write_reg(0x03,0x6);  
					led_ear_sta = 1;
				*/
					//AW9106_i2c_write_reg(0x014,0x3f);
					AW9106_i2c_write_reg(0x012,0x0);	  
					AW9106_i2c_write_reg(0x013,0x0);							
					AW9106_i2c_write_reg(0x11,0x80);
					led_ear_sta = 1;
					break;
				case '0':
					AW9106_i2c_write_reg(0x012,0x3);	  
					AW9106_i2c_write_reg(0x013,0xf);		
	/*
					AW9106_i2c_write_reg(0x02,0x3);    
					AW9106_i2c_write_reg(0x03,0xf);    
					led_ear_sta = 0;
	*/
					//AW9106_i2c_write_reg(0x02,0x3);    
					//AW9106_i2c_write_reg(0x03,0xf);    
					//AW9106_i2c_write_reg(0x014,0x0);		
					//AW9106_i2c_write_reg(0x11,0x02);	//test
	
					led_ear_sta = 0;
					break;
				default:
					return -2;					
			}
		}
		else if(led == 'C' || led == 'c')
		{
			breathleds_i2c_client->addr = 0xb2>>1;
			switch(sta)
			{
				case '1':	
//					AW9106_i2c_write_reg(0x04,0x00);   //OUT4-OUT5自主呼吸BLINK模式使能 	out5/out2 R  out4/out1 G   out3/out0 B	
//					AW9106_i2c_write_reg(0x05,0x09);   //OUT0-OUT3自主呼吸BLINK模式使能					
					//AW9106_i2c_write_reg(0x02,0x3);    
					//AW9106_i2c_write_reg(0x03,0xf); 
					AW9106_i2c_write_reg(0x012,0x0);	  
					AW9106_i2c_write_reg(0x013,0x0);		

					//AW9106_i2c_write_reg(0x014,0x3f);	
					AW9106_i2c_write_reg(0x11,0x80);	 
					led_chest_sta = 1;
					break;
				case '0':

					AW9106_i2c_write_reg(0x012,0x3);	  
					AW9106_i2c_write_reg(0x013,0xf);		
   					//AW9106_i2c_write_reg(0x02,0x3);    
					//AW9106_i2c_write_reg(0x03,0xf); 
					//AW9106_i2c_write_reg(0x014,0x0);		
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
	strcpy(buf, data);

	return 0;
}

static DEVICE_ATTR(onoff, 0664, show_onoff, store_onoff);

/****************************************************************************
 * driver functions
 ***************************************************************************/
static int breathleds_probe(struct platform_device *pdev)
{
	int ret;
	AW9106_Hw_reset();
	
	i2c_register_board_info(BREATHLEDS_I2C_BUSNUM, &breathleds_i2c_hw1, 1);
	i2c_register_board_info(BREATHLEDS_I2C_BUSNUM, &breathleds_i2c_hw2, 1);
	
	i2c_add_driver(&breathleds_i2c_driver);

	return ret;
}


static int breathleds_remove(struct platform_device *pdev)
{
    return 0;
}

static int breathleds_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}
static int breathleds_shutdown(struct platform_device *pdev)
{
	AW9106_Hw_reset();
	return 0;
}

static int breathleds_resume(struct platform_device *pdev)
{
    return 0;
}

#define DEV_NAME   "breathleds"
static struct dev_t *b_dev = NULL;
static struct cdev *breathleds_cdev;
static struct class *breathleds_class = NULL;
struct device *breathleds_dev = NULL;

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
		breathleds_i2c_client->addr = 0xb6>>1;
		switch(sta)
		{
			case '1':
			/*	
				AW9106_i2c_write_reg(0x02,0x3);    
				AW9106_i2c_write_reg(0x03,0x6);  
				led_ear_sta = 1;
			*/
			//	AW9106_i2c_write_reg(0x04,0x01);   //OUT4-OUT5自主呼吸BLINK模式使能		out5/out2 R  out4/out1 G   out3/out0 B  
		 	//	AW9106_i2c_write_reg(0x05,0x02);   //OUT0-OUT3自主呼吸BLINK模式使能						
				AW9106_i2c_write_reg(0x012,0x0);      
				AW9106_i2c_write_reg(0x013,0x0);        
				//AW9106_i2c_write_reg(0x014,0x3f);
				AW9106_i2c_write_reg(0x11,0x80);
				led_ear_sta = 1;
	 			break;
			case '0':
				AW9106_i2c_write_reg(0x012,0x3);      
				AW9106_i2c_write_reg(0x013,0xf);        
				//AW9106_i2c_write_reg(0x02,0x3);    
				//AW9106_i2c_write_reg(0x03,0xf);    
				//AW9106_i2c_write_reg(0x014,0x0);        
				//AW9106_i2c_write_reg(0x11,0x02);

				led_ear_sta = 0;
/*
				AW9106_i2c_write_reg(0x02,0x3);    
				AW9106_i2c_write_reg(0x03,0xf);    
				led_ear_sta = 0;
*/
				break;
			default:
				return -2;					
		}
	}
	else if(led == 'C' || led == 'c')
	{
		breathleds_i2c_client->addr = 0xb2>>1;
		switch(sta)
		{
			case '1':						
	 		//	AW9106_i2c_write_reg(0x04,0x01);   //OUT4-OUT5自主呼吸BLINK模式使能		out5/out2 R  out4/out1 G   out3/out0 B  
	 		//	AW9106_i2c_write_reg(0x05,0x02);   //OUT0-OUT3自主呼吸BLINK模式使能				
				AW9106_i2c_write_reg(0x012,0x0);      
				AW9106_i2c_write_reg(0x013,0x0);        
				//AW9106_i2c_write_reg(0x014,0x3f);      
				AW9106_i2c_write_reg(0x11,0x80);	 
				led_chest_sta = 1;
	 			break;
			case '0':
				AW9106_i2c_write_reg(0x012,0x3);      
				AW9106_i2c_write_reg(0x013,0xf);        
				//AW9106_i2c_write_reg(0x02,0x3);    
				//AW9106_i2c_write_reg(0x03,0xf);    
				//AW9106_i2c_write_reg(0x014,0x0);        
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
	
	return 0;
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

static int breathleds_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	   printk("[%s] breathleds_i2c_probe\n", id->name);

	   /* Kirby: add new-style driver { */
	   breathleds_i2c_client = client;
	   printk("[%x] breathleds_i2c_probe\n", client->addr);
	//	 breathleds_i2c_client->addr = breathleds_i2c_client->addr >> 1;
	   
	   AW9106_init();
	   AW9106_init_pattern();	

	  int err,ret;
		ret = alloc_chrdev_region(&b_dev, 0, 1, DEV_NAME);
		   if (ret< 0) {
		   printk("motor  alloc_chrdev_region failed, %d", ret);
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
			printk("Attatch file motor operation failed, %d", ret);
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
				printk("Failed to create device file(%s)!\n",
					  dev_attr_onoff.attr.name);   
		if (device_create_file(breathleds_dev, &dev_attr_color) < 0)
				printk("Failed to create device file(%s)!\n",
					  dev_attr_color.attr.name);
		if (device_create_file(breathleds_dev, &dev_attr_frequency) < 0)
				printk("Failed to create device file(%s)!\n",
					  dev_attr_frequency.attr.name);

/*
	if(strcmp(g_leds_data[i]->cdev.name, "breathled_ear") == 0) {
		rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_ear_color);
		if (rc) {
			LEDS_DRV_DEBUG("[LED]device_create_file ear_color fail!\n");
		}
	
		rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_ear_onoff);
		if (rc) {
			LEDS_DRV_DEBUG("[LED]device_create_file onofffail!\n");
		}
		*/
/*			
		rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_frequency);
		if (rc) {
			LEDS_DRV_DEBUG("[LED]device_create_file duty fail!\n");
		}
	
		rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_pwm_register);
		if (rc) {
			LEDS_DRV_DEBUG("[LED]device_create_file duty fail!\n");
		}

//				bl_setting = &g_leds_data[i]->cust;

	}
*/

/*
	else if (strcmp(g_leds_data[i]->cdev.name, "breathleds_chest") == 0) {
		rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_duty);
		if (rc) {
			LEDS_DRV_DEBUG("[LED]device_create_file duty fail!\n");
		}
	
		rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_div);
		if (rc) {
			LEDS_DRV_DEBUG("[LED]device_create_file duty fail!\n");
		}
	
		rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_frequency);
		if (rc) {
			LEDS_DRV_DEBUG("[LED]device_create_file duty fail!\n");
		}
	
		rc = device_create_file(g_leds_data[i]->cdev.dev, &dev_attr_pwm_register);
		if (rc) {
			LEDS_DRV_DEBUG("[LED]device_create_file duty fail!\n");
		}
		bl_setting = &g_leds_data[i]->cust;
	}
*/

	return 0;
EXIT:

 	if(breathleds_cdev != NULL)
  	{
		cdev_del(breathleds_cdev);
		breathleds_cdev = NULL;
 	}

     unregister_chrdev_region(breathleds_dev, 1);
	 return -1;
/*****************************************************************************
*****************************************************************************/
}
static int breathleds_i2c_remove(struct i2c_client *client)
{
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

	LEDS_DRV_DEBUG("[LED]%s\n", __func__);
	printk("daviekuo %s 1111111\n", __func__);
	#ifdef CONFIG_HAS_EARLYSUSPEND
	#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&breathleds_early_suspend_handler);
	#endif
	#endif
/*	
	mt_set_gpio_mode((GPIO128 | 0x80000000), GPIO_MODE_00);
	mt_set_gpio_dir((GPIO128 | 0x80000000),GPIO_DIR_OUT);
	mt_set_gpio_out((GPIO128 | 0x80000000),1);
*/
	mt_set_gpio_dir((GPIO21 | 0x80000000), GPIO_DIR_OUT);
	mt_set_gpio_mode((GPIO21 | 0x80000000), GPIO_MODE_00);
		mt_set_gpio_out((GPIO21 | 0x80000000),0);
	ret = platform_device_register(&breathleds_device);
	if (ret)
		printk("breathleds_device_init:dev:E%d\n", ret);
	
	ret = platform_driver_register(&breathleds_driver);

	if (ret) {
		printk("breathleds_init:drv:E%d\n", ret);
		return ret;
	}
	
	return ret;
}

static void __exit breathleds_exit(void)
{
	platform_driver_unregister(&breathleds_driver);
}

module_param(debug_enable_led, int, 0644);

module_init(breathleds_init);
module_exit(breathleds_exit);

MODULE_AUTHOR("MediaTek Inc.");
MODULE_DESCRIPTION("BreathLed driver for MediaTek");
MODULE_LICENSE("GPL");


