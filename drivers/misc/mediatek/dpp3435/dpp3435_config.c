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
//#include "cx20810_config.h"

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>
#include <mach/mt_pm_ldo.h>

//  mem define
//static unsigned int map_io_base;

#define uint08 unsigned char


#define DPP_NAME  "dpp3435"


#define DPP2600_DEV_ADDR	0x88
#define DPP3430_DEV_ADDR                    0x36
#define TVP5150_DEV_ADDR      0xB8
#define WRITE_LED_CURRENT                0x54
#define KEYSTONE_CORRECT                0x88
#define DPP_INPUT_VIDEO               0x05

static struct i2c_client *dpp_client;


static struct i2c_board_info __initdata dpp_dev[]=
{ 	
	{I2C_BOARD_INFO(DPP_NAME, (0x36>>1))},
	
};
#define I2C_NUM 3

static const struct i2c_device_id i2c_driver_dpp_id[]=
{
    {DPP_NAME, 0},
	
    {}
};
   
#define GPIO_PROJ_PIN   (GPIO108|0x80000000)
#define GPIO_FAN_PIN   (GPIO103|0x80000000)
#define GPIO_DPP_POWER_PIN   (GPIO125|0x80000000)
#define GPIO_DPP_POWER_12V  (GPIO21|0x80000000)

#define READ_SIZE    64

 void read_dpp3430_i2c(uint08 addr, uint08 subaddr, uint08* Param, uint08 ParamSize, uint08 *Data, uint08 DataSize)
{
  uint08 i2c_array[READ_SIZE],len;
  uint08 status, i, j,ret;

	len=ParamSize+1;
  // setup the read request as I2C Write
  // Addr, Opcode/SubAddr, Param
  j = 0;
  i2c_array[j] = subaddr;

  if(ParamSize != 0)
  {
	j++;
	for (i=0; i < ParamSize; i++)
	{
		i2c_array[j] = *(Param + i);
		j++;
	}
  }

  for (i=0; i<3; i++) 
	  {
		  ret = i2c_master_send( dpp_client , i2c_array,ParamSize+1);
		  if (ret == len)
			{  printk("**********333\n"); break;}
	  }
  
	dpp_client->addr +=1;
          ret = i2c_master_recv( dpp_client , i2c_array, DataSize);
	  
	  if (ret != DataSize)
	  {
			 printk("**********************   5555	  failed  %s \r\n", __func__);
	  
		  dev_err(&dpp_client->dev,"%s: i2c_master_recv() failed, ret=%d\n",
			  __func__, ret);
	  }
  for (i=0; i<DataSize; i++)
  {
	  *(Data + i) = i2c_array[i];
	  printk("dpp3435=%x\n", i2c_array[i]);
  }

  return;
}

 void write_dpp3430_i2c(uint08 addr, uint08 subaddr, uint08 *Param, uint08 ParamSize)
{
  uint08 status, i, j;
  uint08 i2c_array[12]; // Params max size cap at 12 bytes
  unsigned char rdbuf[1], wrbuf[1], ret,len;

  len=ParamSize+1;
  j=0;
  i2c_array[j] = subaddr;

  if (ParamSize != 0)
  {
	j++;
	for (i=0; i<ParamSize; i++)
	{
		i2c_array[j] = *(Param + i);
		j++;
	}
  }
	
  for (i=0; i<3; i++) 
  {
	  ret = i2c_master_send( dpp_client , i2c_array, len);
	  if (ret == len)
		{
		printk("write_dpp3430_i2c  sucess!!\n"); 
		break;
		}

  }
//  printk("write_dpp3430_i2c  ret=%d,,%x\n",ret,dpp_client->addr);

  return;
}
 void Setting_input_video(int video)
 {
	 uint08 Param[2];
	 Param[0]=video;

	 write_dpp3430_i2c (DPP3430_DEV_ADDR, DPP_INPUT_VIDEO, Param, 1);
 }
void Setting_Image_Correct(uint08 m_throw,uint08 l_throw,uint08 m_DMD,uint08 l_DMD,uint08 l_PP,uint08 m_PP)
{
	uint08 Param[6],buf[5];

	Param[0]=0x01;//enble;
	Param[1]=l_throw;
	Param[2]=m_throw;
	Param[3]=l_DMD;
	Param[4]=m_DMD;

	write_dpp3430_i2c (DPP3430_DEV_ADDR, KEYSTONE_CORRECT, Param, 5);

	buf[0]=l_PP;
	buf[1]=m_PP;
	write_dpp3430_i2c (DPP3430_DEV_ADDR, 0xbb, buf, 2);

}
void Setting_Led_Current(uint08 led_drive_current)
{
	 uint08 Param[8];
     switch(led_drive_current)
     {
         case 1: //8w
	     Param[0] = 0x11; // RED LSB
	     Param[1] = 0x01; // RED MSB

	     Param[2] = 0xEF; // Green LSB
	     Param[3] = 0x01; // Green MSB

	     Param[4] = 0x88; // Blue LSB
	     Param[5] = 0x01; // Blue MSB
         break;

         case 2: //15W
  	     Param[0] = 0x12; // RED LSB
   	     Param[1] = 0x02; // RED MSB

   	     Param[2] = 0x55; // Green LSB
   	     Param[3] = 0x03; // Green MSB

   	     Param[4] = 0xAB; // Blue LSB
   	     Param[5] = 0x02; // Blue MSB
         break;
    
     }

     write_dpp3430_i2c (DPP3430_DEV_ADDR, WRITE_LED_CURRENT, Param, 6);
}

void dpp3438_power_on(void)
{
	 mt_set_gpio_mode(GPIO_DPP_POWER_PIN, GPIO_MODE_00);	
	 mt_set_gpio_dir(GPIO_DPP_POWER_PIN, GPIO_DIR_OUT) ;	 
	 mt_set_gpio_out(GPIO_DPP_POWER_PIN,1);
	
	 mt_set_gpio_mode(GPIO_FAN_PIN, GPIO_MODE_00);  
	mt_set_gpio_dir(GPIO_FAN_PIN, GPIO_DIR_OUT) ;   
	mt_set_gpio_out(GPIO_FAN_PIN,1);
	
	mt_set_gpio_mode(GPIO_PROJ_PIN, GPIO_MODE_00);	
	 mt_set_gpio_dir(GPIO_PROJ_PIN, GPIO_DIR_OUT) ;	
	mt_set_gpio_out(GPIO_PROJ_PIN,1);
	
	 mt_set_gpio_mode(GPIO_DPP_POWER_12V, GPIO_MODE_00);  
	   mt_set_gpio_dir(GPIO_DPP_POWER_12V, GPIO_DIR_OUT) ;	
	   mt_set_gpio_out(GPIO_DPP_POWER_12V,1);

	   Setting_Led_Current(2);//15w

}
void dpp3438_power_off(void)
{
	mt_set_gpio_out(GPIO_PROJ_PIN,0);
	mt_set_gpio_out(GPIO_DPP_POWER_PIN,0);
	mt_set_gpio_out(GPIO_DPP_POWER_12V,0);
	mt_set_gpio_out(GPIO_FAN_PIN,0);
}

static int i2c_driver_dpp_probe(struct i2c_client * client, const struct i2c_device_id* id)
{
	printk("i2c_driver_dpp_probe=%x\n",client->addr);
	
 	dpp_client=client;

#if 0
	 mt_set_gpio_mode(GPIO_DPP_POWER_PIN, GPIO_MODE_00);  
	 mt_set_gpio_dir(GPIO_DPP_POWER_PIN, GPIO_DIR_OUT) ;   
	 mt_set_gpio_out(GPIO_DPP_POWER_PIN,1);

	 mt_set_gpio_mode(GPIO_FAN_PIN, GPIO_MODE_00);  
	 mt_set_gpio_dir(GPIO_FAN_PIN, GPIO_DIR_OUT) ;   
	 mt_set_gpio_out(GPIO_FAN_PIN,1);

	 mt_set_gpio_mode(GPIO_PROJ_PIN, GPIO_MODE_00);	
	mt_set_gpio_dir(GPIO_PROJ_PIN, GPIO_DIR_OUT) ;   
	  mt_set_gpio_out(GPIO_PROJ_PIN,1);

	mt_set_gpio_mode(GPIO_DPP_POWER_12V, GPIO_MODE_00);  
	   mt_set_gpio_dir(GPIO_DPP_POWER_12V, GPIO_DIR_OUT) ;	
	   mt_set_gpio_out(GPIO_DPP_POWER_12V,1);

#endif	 
 	Setting_Led_Current(1);
	

    return 0;
}

static int i2c_driver_dpp_remove(struct i2c_client * client)
{
    printk("Timothy:cx20810.c->i2c_driver_cx20810_remove()\n");
    return 0;
}

static struct i2c_driver i2c_driver_dpp=
{
    .probe          = i2c_driver_dpp_probe,
    .remove         = i2c_driver_dpp_remove,
    .id_table       = i2c_driver_dpp_id,
    .driver         =
    {
        .name   = DPP_NAME,
        .owner  = THIS_MODULE,
    },

};

static int __init i2c_driver_dpp_init(void)
{
	int ret;
    printk("i2c_driver_dpp_init()\n");
	
  i2c_register_board_info(I2C_NUM, dpp_dev, 1);

    return i2c_add_driver(&i2c_driver_dpp);

}

static void __exit i2c_driver_dpp_exit(void)
{
    printk("Timothy:cx20810.c->i2c_driver_cx20810_exit()\n");
    i2c_del_driver(&i2c_driver_dpp);
}



module_init(i2c_driver_dpp_init);
module_exit(i2c_driver_dpp_exit);

MODULE_AUTHOR("Timothy");
MODULE_DESCRIPTION("I2C device dpp loader");
MODULE_LICENSE("GPL");
