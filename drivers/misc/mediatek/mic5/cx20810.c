#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
//#include <mach/gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>

// codec init param
#include "cx20810_config.h"

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>
#include <mach/mt_pm_ldo.h>
#include <linux/wakelock.h>
#include <linux/earlysuspend.h>

//  mem define
//static unsigned int map_io_base;
static void __iomem *map_io_base;

#define D_IO_BASE   map_io_base
#define D_IO_BASE_PHY  (0x11240000)// (0x01c20800)
#define D_IO_LEN    (0x400)
#define D_IO_A  (0)
#define D_IO_B  (1)
#define D_IO_C  (2)
#define D_IO_D  (3)
#define D_IO_E  (4)
#define D_IO_F  (5)
#define D_IO_G  (6)
#define D_IO_H  (7)
#define D_IO_I  (8)
#define D_IO_CFG0(x)    (D_IO_BASE+(x*0x24)+0x00)
#define D_IO_CFG1(x)    (D_IO_BASE+(x*0x24)+0x04)
#define D_IO_CFG2(x)    (D_IO_BASE+(x*0x24)+0x08)
#define D_IO_CFG3(x)    (D_IO_BASE+(x*0x24)+0x0c)
#define D_IO_DAT(x)     (D_IO_BASE+(x*0x24)+0x10)
#define D_IO_DRV0(x)    (D_IO_BASE+(x*0x24)+0x14)
#define D_IO_DRV1(x)    (D_IO_BASE+(x*0x24)+0x18)
#define D_IO_PUL0(x)    (D_IO_BASE+(x*0x24)+0x1c)
#define D_IO_PUL1(x)    (D_IO_BASE+(x*0x24)+0x20)

#define I2C_CX20810_DRIVER_NAME "i2c_cx20810"
#define I2C_CX20810_DRIVER_NAME1 "i2c_cx20810_1"

#define GPIO44_3_3V_PIN (GPIO44|0x80000000)   //ADC POWER
#define GPIO60_LDO_PIN (GPIO60|0x80000000)   //ADC POWER
#define GPIO171_ADC_RST1_PIN (GPIO171|0x80000000)   //low power 3b
#define GPIO128_ADC_RST2_PIN (GPIO128|0x80000000)   //high power 35
#define GPIO129_FPGA_DAT_PIN (GPIO129|0x80000000)    

#define MAX_CX20810_NUM (3)

// g_client_cx20810[0] is on adapter 0 and its address is 0x35
// g_client_cx20810[1] is on adapter 1 and its address is 0x35
// g_client_cx20810[2] is on adapter 1 and its address is 0x3B
static struct i2c_client * g_client_cx20810[MAX_CX20810_NUM];
static const unsigned short i2c_cx20810_addr[] = {(0x35), (0x3B), I2C_CLIENT_END};
static struct i2c_board_info __initdata cx20810_dev[]=
{ 	
	{I2C_BOARD_INFO(I2C_CX20810_DRIVER_NAME1, (0x3b))},
	{I2C_BOARD_INFO(I2C_CX20810_DRIVER_NAME, (0x35))},
};
#define I2C_NUM 3

static const struct i2c_device_id i2c_driver_cx20810_id[]=
{
    {I2C_CX20810_DRIVER_NAME, 0},
	{I2C_CX20810_DRIVER_NAME1, 0},
    {}
};

// function declaration
static int cx20810_hw_init();
static int i2c_driver_cx20810_probe(struct i2c_client * client, const struct i2c_device_id* id);
static int i2c_driver_cx20810_remove(struct i2c_client * client);
static int i2c_driver_cx20810_detect(struct i2c_client * client, struct i2c_board_info * info);
static int i2c_master_send_array_to_cx20810(const struct i2c_client *client, const char *buf, int length);
static void cx20810_init(int index, int mode);
int cx20810_set_mode(int mode, int index);
int testflag=0;
// set cx20810 work mode  
int cx20810_set_mode(int mode, int index)
{
    //printk("Timothy:cx20810.c->cx20810_set_mode(), mode = %d, index = %d\n", mode, index);
    int i;
    int ret;
    char * param;
    int length;


    switch(mode)
    {
        case CX20810_NORMAL_MODE:
            param = codec_config_param_normal_mode;
            length = sizeof(codec_config_param_normal_mode);
            break;
        case CX20810_NORMAL_MODE_SIMPLE:
            param = codec_config_param_normal_mode_simple;
            length = sizeof(codec_config_param_normal_mode_simple);
            break;
        case CX20810_48K_16BIT_MODE:
            param = codec_config_param_48k_16bit_mode;
            length = sizeof(codec_config_param_48k_16bit_mode);
            break;
        case CX20810_96K_16BIT_MODE:
            param = codec_config_param_96k_16bit_mode;
            length = sizeof(codec_config_param_96k_16bit_mode);
            break;
        case CX20810_NIRMAL_MODE_CODEC3:
            param = codec3_config_param_normal_mode;
            length = sizeof(codec3_config_param_normal_mode);
            break;
        case CX20810_NIRMAL_MODE_CODEC3_SIMPLE:
            param = codec3_config_param_normal_mode_simple;
            length = sizeof(codec3_config_param_normal_mode_simple);
            break;
        default:
            return ;
            break;
    }

    // if client is null, return
    if(g_client_cx20810[index] == NULL)
    {
        printk("Timothy:cx20810(%d) is not detected yet\n", index);
        return -1;
    }

    ret = i2c_master_send_array_to_cx20810(g_client_cx20810[index], param, length);
    if(ret != 0)
    {
        printk("Timothy:cx82011[%x] init error!\n", g_client_cx20810[index]->addr);
        return -1;
    }
    else
    {
        printk("Timothy:cx20810[%x] init ok\n", g_client_cx20810[index]->addr);
		
        return 0;
    }
}
EXPORT_SYMBOL(cx20810_set_mode);

// send parameters to cx20810 as master
 unsigned char ADC_i2c_read_reg(unsigned char regaddr,int idex) 
{
	unsigned char rdbuf[1], wrbuf[1], ret, i;

	wrbuf[0] = regaddr;

	for (i=0; i<3; i++) 
	{
		ret = i2c_master_send( g_client_cx20810[idex] , wrbuf, 1);
		if (ret == 1)
			break;
	}
	
	ret = i2c_master_recv( g_client_cx20810[idex] , rdbuf, 1);
	
	if (ret != 1)
	{
		   printk("**********************   5555    failed  %s \r\n", __func__);
	
		dev_err(&g_client_cx20810[idex]->dev,"%s: i2c_master_recv() failed, ret=%d\n",
			__func__, ret);
	}
	
    	return rdbuf[0];
		
}

static int i2c_master_send_array_to_cx20810(const struct i2c_client *client, const char *buf, int length)
{
    printk("Timothy:cx20810.c->i2c_master_send_array_to_cx20810()\n");
    int i;
    int nwrite;
    for(i = 0; i < (length / 2); i ++)
    {
        nwrite = i2c_master_send(client, buf + i * 2, 2);
        if(nwrite != 2)
        {     
            printk("Timothy:send to cx20810 error\n");
            return -1;
        }
    }
    return 0;
}
int reset_mic_cx20810(void)
{
	int ret1 ,ret2;
	mt_set_gpio_out(GPIO44_3_3V_PIN,0);
	mdelay(100);
	mt_set_gpio_out(GPIO44_3_3V_PIN,1);
	mdelay(200);

	cx20810_set_mode(0,0);
	ret1=ADC_i2c_read_reg(0x10,0);
	printk("111add35=%x\n", ret1);
	mdelay(650);
	cx20810_set_mode(0,1);
	ret2=ADC_i2c_read_reg(0x10,1);
	printk("111add3b=%x\n",ret2 );

	if(ret1 ==0x5f && ret2 == 0x5f)
		return 1;
         return 0;
}
// initial cx20810
static void cx20810_init(int index, int mode)
{
    //printk("Timothy:cx20810.c->cx20810_init()\n");
    if(cx20810_set_mode(mode, index) == 0)
    {
        printk(KERN_ERR"KERN_ERR Timothy:cx20810 init success\n");
    }
    else
    {
        printk(KERN_ERR"KERN_ERR Timothy:cx20810 init fail\n");
    }
}

// cx20810 hardware initial(include io)
static int cx20810_hw_init()
{
    //printk("Timothy:cx20810.c->cx20810_hw_init()\n");
#if 0
  if(!D_IO_BASE)
    {
        printk("Timothy:D_IO_BASE is not initial yet, initial it now\n");
        D_IO_BASE = ioremap(D_IO_BASE_PHY, D_IO_LEN);
        if(!D_IO_BASE)
        {
            printk("cx20810 hardware init io error\n");
            return -1;
        }
    }
  #endif
    return 0;
}

static int i2c_driver_cx20810_probe(struct i2c_client * client, const struct i2c_device_id* id)
{
	printk("Timothy:cx20810.c->i2c_driver_cx20810_probe(),client->addr=0x%x  %d\n",client->addr,client->adapter->nr);
	
  if(client->adapter->nr == 3 && client->addr == i2c_cx20810_addr[0])
    {
            mt_set_gpio_out(GPIO128_ADC_RST2_PIN,1);
	 mdelay(30);
	  mt_set_gpio_out(GPIO128_ADC_RST2_PIN,0);
	  mdelay(30);
  	  mt_set_gpio_out(GPIO128_ADC_RST2_PIN,1);
	  mdelay(30);
           g_client_cx20810[0] = client;
          cx20810_init(0, CX20810_NORMAL_MODE);
	printk("111add35=%x\n",ADC_i2c_read_reg(0x10,0) );
    }
    else if(client->adapter->nr == 3 && client->addr == i2c_cx20810_addr[1])  //0x3b--low vol
    {       
 	   mt_set_gpio_out(GPIO171_ADC_RST1_PIN,1);
	   mdelay(30);
            g_client_cx20810[1] = client;
            cx20810_init(1, CX20810_NORMAL_MODE);
	printk("111add3b=%x\n",ADC_i2c_read_reg(0x10,1) );
    }

    return 0;
}

static int i2c_driver_cx20810_remove(struct i2c_client * client)
{
    printk("Timothy:cx20810.c->i2c_driver_cx20810_remove()\n");
    return 0;
}
MODULE_DEVICE_TABLE(i2c, i2c_driver_cx20810_id);

static int i2c_driver_cx20810_detect(struct i2c_client * client, struct i2c_board_info * info)
{
    //printk("Timothy:cx20810.c->i2c_driver_cx20810_detect()...\n");
    struct i2c_adapter * p_adapter;
    const char *type_name = I2C_CX20810_DRIVER_NAME;
    p_adapter = client->adapter;
    printk("Timothy:adapter->nr = %d\n", p_adapter->nr);
    if(3 == p_adapter->nr)
    {
        if(info->addr == i2c_cx20810_addr[0])
        {
            printk("Timothy:detect cx20810 (%x) on i2c adapter (%d)\n", info->addr, p_adapter->nr);
            strlcpy(info->type, type_name, I2C_NAME_SIZE);
            return 0;
        }
        else if(info->addr == i2c_cx20810_addr[1])
        {
            printk("Timothy:detect cx20810 (%x) on i2c adapter (%d)\n", info->addr, p_adapter->nr);
            strlcpy(info->type, type_name, I2C_NAME_SIZE);
            return 0;
        }
    }
    return ENODEV;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_EARLYSUSPEND
int yyd_lock_system=false;

  static void reset_cx2008(void)
{
	mt_set_gpio_out(GPIO128_ADC_RST2_PIN,1);
  	 mdelay(30);
	mt_set_gpio_out(GPIO128_ADC_RST2_PIN,0);
	mdelay(30);
	mt_set_gpio_out(GPIO128_ADC_RST2_PIN,1);
	mdelay(30);
	cx20810_init(0, CX20810_NORMAL_MODE);
	  printk("111add35=%x\n",ADC_i2c_read_reg(0x10,0) );

	  mt_set_gpio_out(GPIO171_ADC_RST1_PIN,1);
	  mdelay(30);
	  mt_set_gpio_out(GPIO171_ADC_RST1_PIN,0);
	   mdelay(30);
	   mt_set_gpio_out(GPIO171_ADC_RST1_PIN,1);
	   mdelay(30);  
            cx20810_init(1, CX20810_NORMAL_MODE);
	printk("111add3b=%x\n",ADC_i2c_read_reg(0x10,1) );

}
  static void m_suspend( struct early_suspend *h )
  {
  	if(yyd_lock_system)
	 mt_set_gpio_out(GPIO44_3_3V_PIN,0);
  }
  
  static void m_resume( struct early_suspend *h )
  {
 	 if(yyd_lock_system)
 	 {
 	 mt_set_gpio_out(GPIO44_3_3V_PIN,1);
	 mdelay(10);
	reset_cx2008();
 	 }
  }
  
  static struct early_suspend misc_early_suspend_handler = {
	  .level = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 1,
	  .suspend = m_suspend,
	  .resume = m_resume,
  };
  
#endif
#endif

static struct i2c_driver i2c_driver_cx20810=
{
  //  .class          = I2C_CLASS_HWMON,
    .probe          = i2c_driver_cx20810_probe,
    .remove         = i2c_driver_cx20810_remove,
    .id_table       = i2c_driver_cx20810_id,
    .driver         =
    {
        .name   = I2C_CX20810_DRIVER_NAME,
        .owner  = THIS_MODULE,
    },
//   .detect         = i2c_driver_cx20810_detect,
  //.address_list   = i2c_cx20810_addr
};

static int __init i2c_driver_cx20810_init(void)
{
	int ret;
    printk("Timothy:cx20810.c->i2c_driver_cx20810_init()\n");
 //   cx20810_hw_init();
 
 int a=1;
	 if(a)
	 {
	 hwPowerOn(MT6328_POWER_LDO_VMC, VOL_3300, "3v3msdc" );//

	  mt_set_gpio_mode(GPIO44_3_3V_PIN, GPIO_MODE_00);  
	 mt_set_gpio_dir(GPIO44_3_3V_PIN, GPIO_DIR_OUT) ;   
	 mt_set_gpio_out(GPIO44_3_3V_PIN,1);

	 mt_set_gpio_mode(GPIO60_LDO_PIN, GPIO_MODE_00);  
	 mt_set_gpio_dir(GPIO60_LDO_PIN, GPIO_DIR_OUT) ;   
	 mt_set_gpio_out(GPIO60_LDO_PIN,1);

	 mt_set_gpio_mode(GPIO171_ADC_RST1_PIN, GPIO_MODE_00);  
	 mt_set_gpio_dir(GPIO171_ADC_RST1_PIN, GPIO_DIR_OUT) ;   
	 mt_set_gpio_out(GPIO171_ADC_RST1_PIN,0);
 
	 mt_set_gpio_mode(GPIO128_ADC_RST2_PIN, GPIO_MODE_00);	
	 mt_set_gpio_dir(GPIO128_ADC_RST2_PIN, GPIO_DIR_OUT) ;	 
	 mt_set_gpio_out(GPIO128_ADC_RST2_PIN,0);

	  mt_set_gpio_mode(GPIO129_FPGA_DAT_PIN, GPIO_MODE_00);	
	 mt_set_gpio_dir(GPIO129_FPGA_DAT_PIN, GPIO_DIR_OUT) ;	 
	 mt_set_gpio_out(GPIO129_FPGA_DAT_PIN,0);
 
	 a=0;
	 }
  i2c_register_board_info(3, cx20810_dev, 2);

	#ifdef CONFIG_HAS_EARLYSUSPEND
	#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&misc_early_suspend_handler);
	#endif
	#endif

    return i2c_add_driver(&i2c_driver_cx20810);

}

static void __exit i2c_driver_cx20810_exit(void)
{
    printk("Timothy:cx20810.c->i2c_driver_cx20810_exit()\n");
    i2c_del_driver(&i2c_driver_cx20810);
}



module_init(i2c_driver_cx20810_init);
module_exit(i2c_driver_cx20810_exit);

MODULE_AUTHOR("Timothy");
MODULE_DESCRIPTION("I2C device cx20810 loader");
MODULE_LICENSE("GPL");
