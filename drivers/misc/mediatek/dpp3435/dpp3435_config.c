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
#include<linux/kthread.h>

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>
#include <mach/mt_pm_ldo.h>

#include <linux/wait.h>
#include <linux/wakelock.h>
#include <mach/battery_common.h>

#include <misc.h>		//added by daviekuo 0622

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

 DECLARE_WAIT_QUEUE_HEAD(dpp3438_thread_wq);
 uint08 key_switch_flag=0;
 
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
#define GPIO_HDMIPOWER_PIN   (GPIO120|0x80000000)
#define GPIO_JTCK_PIN   (GPIO71|0x80000000)  //en pin

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

 int write_dpp3430_i2c(uint08 addr, uint08 subaddr, uint08 *Param, uint08 ParamSize)
{
  uint08 status, i, j;
  uint08 i2c_array[12]; // Params max size cap at 12 bytes
  unsigned char rdbuf[1], wrbuf[1], len;
 int ret=-1;
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
	  else 
	  	printk("write_dpp3430_i2c  fail   subaddr=0x%x \n",subaddr); 
  }
//  printk("write_dpp3430_i2c  ret=%d,,%x\n",ret,dpp_client->addr);

  return ret;
}
int Setting_input_video(int video)
 {
	 uint08 Param[2];
	 int ret=0;
	 Param[0]=video;
	
	 ret=write_dpp3430_i2c (DPP3430_DEV_ADDR, DPP_INPUT_VIDEO, Param, 1);
	 return ret;
 }
int Setting_Image_Correct(uint08 m_throw,uint08 l_throw,uint08 m_DMD,uint08 l_DMD,uint08 l_PP,uint08 m_PP)
{
	uint08 Param[6],buf[5];
	int ret=-1;
	uint08 enble=1;

	write_dpp3430_i2c (DPP3430_DEV_ADDR, 0x1a, &enble, 1);
	enble=0;
	msleep(100);	
	Param[0]=0x01;//enble;
	Param[1]=0x78;//l_throw;
	Param[2]=0x01;//m_throw;
	Param[3]=0;//l_DMD;
	Param[4]=0;//m_DMD;

	write_dpp3430_i2c (DPP3430_DEV_ADDR, KEYSTONE_CORRECT, Param, 5);

	buf[0]=l_PP;
	buf[1]=m_PP;
	ret=write_dpp3430_i2c (DPP3430_DEV_ADDR, 0xbb, buf, 2);

	msleep(100);
	write_dpp3430_i2c (DPP3430_DEV_ADDR, 0x1a, &enble, 1);

	return ret;
}
int Seting_Image_Rotation(int val)
{
	uint08 buf[1];
	int ret=-1;
	buf[0]=val;//0x06;
	ret=write_dpp3430_i2c(DPP3430_DEV_ADDR, 0x14, buf,1);
	return ret;
}
int Setting_Led_Current(uint08 led_drive_current)
{
	 uint08 Param[8],ret=0;
	 uint08 LOOK[3];
     switch(led_drive_current)
     {
       case 0: // test
	     Param[0] = 0x41; // RED LSB
	     Param[1] = 0x00; // RED MSB

	     Param[2] = 0x4F; // Green LSB
	     Param[3] = 0x00; // Green MSB

	     Param[4] = 0x48; // Blue LSB
	     Param[5] = 0x00; // Blue MSB
	     
	     LOOK[0]=0x01;
         break;
         case 1: //8w
	     Param[0] = 0x11; // RED LSB
	     Param[1] = 0x01; // RED MSB

	     Param[2] = 0xEF; // Green LSB
	     Param[3] = 0x01; // Green MSB

	     Param[4] = 0x88; // Blue LSB
	     Param[5] = 0x01; // Blue MSB
	     
	     LOOK[0]=0x01;
         break;

         case 2: //15W
  	     Param[0] = 0x12; // RED LSB
   	     Param[1] = 0x02; // RED MSB

   	     Param[2] = 0x55; // Green LSB
   	     Param[3] = 0x03; // Green MSB

   	     Param[4] = 0xAB; // Blue LSB
   	     Param[5] = 0x02; // Blue MSB
   	     
   	     LOOK[0]=0x02;
         break;
	  case 3: //13.5W
  	     Param[0] = 0x08; // RED LSB
   	     Param[1] = 0x02; // RED MSB

   	     Param[2] = 0x14; // Green LSB
   	     Param[3] = 0x03; // Green MSB

   	     Param[4] = 0x8A; // Blue LSB
   	     Param[5] = 0x02; // Blue MSB
   	     
   	     LOOK[0]=0x02;
         break;
    
     }

     ret= write_dpp3430_i2c (DPP3430_DEV_ADDR, WRITE_LED_CURRENT, Param, 6);
	  if(ret <0) return -1;
     ret=  write_dpp3430_i2c (DPP3430_DEV_ADDR, 0x22, LOOK, 1);
	   if(ret <0) return -1;
    return ret;	   
}

void dpp3438_power_on(void)
{
	// mt_set_gpio_mode(GPIO_DPP_POWER_PIN, GPIO_MODE_00);	
	// mt_set_gpio_dir(GPIO_DPP_POWER_PIN, GPIO_DIR_OUT) ;	 
	// mt_set_gpio_out(GPIO_DPP_POWER_PIN,1);
	 
	mt_set_gpio_mode(GPIO_DPP_POWER_12V, GPIO_MODE_00);  
	 mt_set_gpio_dir(GPIO_DPP_POWER_12V, GPIO_DIR_OUT) ;  
	 mt_set_gpio_out(GPIO_DPP_POWER_12V,1);
			  
	mt_set_gpio_mode(GPIO_PROJ_PIN, GPIO_MODE_00);	
	 mt_set_gpio_dir(GPIO_PROJ_PIN, GPIO_DIR_OUT) ;	
	mt_set_gpio_out(GPIO_PROJ_PIN,1);

	// mt_set_gpio_mode(GPIO_FAN_PIN, GPIO_MODE_00);  
	//mt_set_gpio_dir(GPIO_FAN_PIN, GPIO_DIR_OUT) ;   
	//mt_set_gpio_out(GPIO_FAN_PIN,1);


}
void dpp3438_power_off(void)
{
	mt_set_gpio_out(GPIO_PROJ_PIN,0);
	//mt_set_gpio_out(GPIO_DPP_POWER_PIN,0);
	mt_set_gpio_out(GPIO_DPP_POWER_12V,0);
	mt_set_gpio_out(GPIO_FAN_PIN,0);
	
}

int dpp3438_logo_show(void)
{
	char temp[1],i=0,ret=0;
	temp[0]=0;
	ret=write_dpp3430_i2c (0x36, 0x0d, temp, 1);
	 if(ret <0) return -1;
	ret=Setting_input_video(2);
	 if(ret <0) return -1;
	mdelay(30);
	temp[0]=0;
	ret=write_dpp3430_i2c (0x36, 0x35, temp, 0);  
	 if(ret <0) return -1;
	for(i=0;i<6;i++)
	{
		ret=Setting_input_video(0);
		if(ret >0)break;
	}
	return ret;

}

bool hdmi_open_flag=true;
int dpp3438_status_flag=0;
int dpp3438_power_rate=3;

int dpp3438_init(void)
{
	int ret=0;
	dpp3438_power_on();
	msleep(1800);			
	ret=dpp3438_logo_show();
	 if(ret <0) return -1;
	ret=Setting_Led_Current(dpp3438_power_rate);//13.5w

	return ret;
}
static  int dpp_thread_kthread(void *x)
{
	int i=0,ret=0;
	while(1)
	{
		  wait_event(dpp3438_thread_wq, (key_switch_flag == 1));
		  key_switch_flag=0;
		  if(hdmi_open_flag &&( BMT_status.UI_SOC >30 ||1 == mt_get_gpio_in(CHG_DET_PIN)))//11.2v
		  {	
		  	for(i=0;i<4;i++)
		  	{
				ret=dpp3438_init();
				 if(ret <0)
				 {
					 dpp3438_power_off();
					 msleep(200);
				 }
				 else break;				 	
		  	}
			hdmi_open_flag=false;
			dpp3438_status_flag |=0x1;

		         if((dpp3438_status_flag&0x02) !=0x02) //en pin reset
		     	{
		     	   mt_set_gpio_out(GPIO_JTCK_PIN,0);
				   msleep(100);
			    mt_set_gpio_out(GPIO_JTCK_PIN,1);	   
			 }

			 
			 mt_set_gpio_mode(GPIO_FAN_PIN, GPIO_MODE_00);  
			mt_set_gpio_dir(GPIO_FAN_PIN, GPIO_DIR_OUT) ;   
			mt_set_gpio_out(GPIO_FAN_PIN,1);

		  }
		  else
		  {
			  dpp3438_power_off();
			   hdmi_open_flag=true;
			   dpp3438_status_flag &=0xfe;
		  }
		  printk("lllllllll%d\n",hdmi_open_flag);
	}
	return 0;
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

	Setting_Led_Current(1);

#endif	 
	
	kthread_run(dpp_thread_kthread, NULL, "dpp_thread_kthread");


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
	
	mt_set_gpio_mode(GPIO_DPP_POWER_PIN, GPIO_MODE_00);  
	mt_set_gpio_dir(GPIO_DPP_POWER_PIN, GPIO_DIR_OUT) ;   
	mt_set_gpio_out(GPIO_DPP_POWER_PIN,1);

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
