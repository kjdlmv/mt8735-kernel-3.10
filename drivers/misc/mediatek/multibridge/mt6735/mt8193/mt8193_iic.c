/*****************************************************************************/
/* Copyright (c) 2009 NXP Semiconductors BV                                  */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation, using version 2 of the License.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307       */
/* USA.                                                                      */
/*                                                                           */
/*****************************************************************************/
#if defined(CONFIG_MTK_MULTIBRIDGE_SUPPORT)

#define pr_fmt(fmt) "mt8193-iic: " fmt

#include <generated/autoconf.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/vmalloc.h>
#include <linux/disp_assert_layer.h>

#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <asm/io.h>

#include <mach/dma.h>
#include <mach/irqs.h>

#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <asm/tlbflush.h>
#include <asm/page.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>

#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <asm/io.h>

#include <mach/dma.h>
#include <mach/irqs.h>
#include <linux/vmalloc.h>

#include <asm/uaccess.h>

#include "mt8193_iic.h"
#include "cust_mt8193.h"
#include "hdmi_to_dpp3438.h"

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>
#include <misc.h>		//added by daviekuo 0622

/*----------------------------------------------------------------------------*/
/* mt8193 device information                                                  */
/*----------------------------------------------------------------------------*/
#define MAX_TRANSACTION_LENGTH 8
#define MT8193_DEVICE_NAME            "mtk-multibridge"
#define MT8193_I2C_SLAVE_ADDR       0x3A
#define MT8193_I2C_DEVICE_ADDR_LEN   2

#define DEV_NAME   "hdmi_ctl"
static struct cdev *hdmi_cdev;
static dev_t m_dev;
static struct class *hdmi_class = NULL;
struct device *h_device = NULL;

/*----------------------------------------------------------------------------*/
static int mt8193_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int mt8193_i2c_remove(struct i2c_client *client);
static struct i2c_client *mt8193_i2c_client;
static const struct i2c_device_id mt8193_i2c_id[] = {{MT8193_DEVICE_NAME, 0}, {} };
static struct i2c_board_info i2c_mt8193 __initdata = {I2C_BOARD_INFO(MT8193_DEVICE_NAME, (MT8193_I2C_SLAVE_ADDR>>1))};
/*----------------------------------------------------------------------------*/
struct i2c_driver mt8193_i2c_driver = {
	.probe		= mt8193_i2c_probe,
	.remove		= mt8193_i2c_remove,
	.driver		= { .name = MT8193_DEVICE_NAME, },
	.id_table	= mt8193_i2c_id,
};

struct mt8193_i2c_data {
	struct i2c_client *client;
	uint16_t addr;
	int use_reset;		/*use RESET flag*/
	int use_irq;		/*use EINT flag*/
	int retry;
};

static struct mt8193_i2c_data *obj_i2c_data;

/*----------------------------------------------------------------------------*/
int mt8193_i2c_read(u16 addr, u32 *data)
{
	u8 rxBuf[8] = {0};
	int ret = 0;
	struct i2c_client *client = mt8193_i2c_client;
	u8 lens;

	if (((addr >> 8) & 0xFF) >= 0x80) {
		/* 8 bit : fast mode */
		rxBuf[0] = (addr >> 8) & 0xFF;
		lens = 1;
	} else {
		/* 16 bit : noraml mode */
		rxBuf[0] = (addr >> 8) & 0xFF;
		rxBuf[1] = addr & 0xFF;
		lens = 2;
	}

	client->addr = (client->addr & I2C_MASK_FLAG);
	client->timing = 400;
	client->ext_flag = I2C_WR_FLAG;

	ret = i2c_master_send(client, (const char *)&rxBuf, (4 << 8) | lens);
	if (ret < 0) {
		pr_err("%s: read error\n", __func__);
		return -EFAULT;
	}
	*data = (rxBuf[3] << 24) | (rxBuf[2] << 16) | (rxBuf[1] << 8) | (rxBuf[0]); /*LSB fisrt*/
	return 0;
}
/*----------------------------------------------------------------------------*/
EXPORT_SYMBOL_GPL(mt8193_i2c_read);
/*----------------------------------------------------------------------------*/

int mt8193_i2c_write(u16 addr, u32 data)
{
	struct i2c_client *client = mt8193_i2c_client;
	u8 buffer[8];
	int ret = 0;
	struct i2c_msg msg = {
		.addr	= client->addr & I2C_MASK_FLAG,
		.flags	= 0,
		.len	= (((addr >> 8) & 0xFF) >= 0x80)?5:6,
		.buf	= buffer,
		.timing	= 400,
	};

	if (((addr >> 8) & 0xFF) >= 0x80) {
		/* 8 bit : fast mode */
		buffer[0] = (addr >> 8) & 0xFF;
		buffer[1] = (data >> 24) & 0xFF;
		buffer[2] = (data >> 16) & 0xFF;
		buffer[3] = (data >> 8) & 0xFF;
		buffer[4] = data & 0xFF;
	} else {
		/* 16 bit : noraml mode */
		buffer[0] = (addr >> 8) & 0xFF;
		buffer[1] = addr & 0xFF;
		buffer[2] = (data >> 24) & 0xFF;
		buffer[3] = (data >> 16) & 0xFF;
		buffer[4] = (data >> 8) & 0xFF;
		buffer[5] = data & 0xFF;
	}

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		pr_err("%s: send command error\n", __func__);
		return -EFAULT;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
EXPORT_SYMBOL_GPL(mt8193_i2c_write);

/*----------------------------------------------------------------------------*/
/* IIC Probe                                                                  */
/*----------------------------------------------------------------------------*/
static int mt8193_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = -1;
	struct mt8193_i2c_data *obj;

	pr_notice("%s\n", __func__);
	printk("%s\n", __func__);

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (obj == NULL) {
		ret = -ENOMEM;
		pr_err("%s: Allocate ts memory fail\n", __func__);
		return ret;
	}
	obj_i2c_data = obj;
	client->timing = 400;
	obj->client = client;
	mt8193_i2c_client = obj->client;
	i2c_set_clientdata(client, obj);

	return 0;
}
/*----------------------------------------------------------------------------*/

static int mt8193_i2c_remove(struct i2c_client *client)
{
	pr_info("%s\n", __func__);
	mt8193_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}

/*----------------------------------------------------------------------------*/
/* device driver probe                                                        */
/*----------------------------------------------------------------------------*/
#define GPIO_JTCK_PIN   (GPIO71|0x80000000)
#define GPIO_JTDI_PIN   (GPIO72|0x80000000)
#define GPIO_JTDO_PIN   (GPIO73|0x80000000)
#define GPIO_HDMIPOWER_PIN   (GPIO120|0x80000000)

#define GPIO_101   (GPIO101|0x80000000)
#define GPIO_102   (GPIO102|0x80000000)
#define GPIO_119   (GPIO119|0x80000000)


static int mt8193_probe(struct platform_device *pdev)
{
	pr_notice("%s\n", __func__);

#if 1  //for test
		mt_set_gpio_mode(GPIO_JTCK_PIN, GPIO_MODE_00);	///TEST
		mt_set_gpio_dir(GPIO_JTCK_PIN, GPIO_DIR_OUT) ;	 
		mt_set_gpio_out(GPIO_JTCK_PIN,1);
			
		mt_set_gpio_mode(GPIO_JTDI_PIN, GPIO_MODE_00);	
		mt_set_gpio_dir(GPIO_JTDI_PIN, GPIO_DIR_OUT) ;	 
		mt_set_gpio_out(GPIO_JTDI_PIN,0);
		
		mt_set_gpio_mode(GPIO_JTDO_PIN, GPIO_MODE_00);	
		mt_set_gpio_dir(GPIO_JTDO_PIN, GPIO_DIR_OUT) ;	 
		mt_set_gpio_out(GPIO_JTDO_PIN,0);
	
		mt_set_gpio_mode(GPIO_HDMIPOWER_PIN, GPIO_MODE_00);  
		mt_set_gpio_dir(GPIO_HDMIPOWER_PIN, GPIO_DIR_OUT) ;   
		mt_set_gpio_out(GPIO_HDMIPOWER_PIN,1);

		
		//mt_set_gpio_mode(GPIO_BAT_ID, GPIO_MODE_00);  //hdmi ic(mt8193 3.3V 1.2V)  power
		//mt_set_gpio_dir(GPIO_BAT_ID, GPIO_DIR_OUT) ;   
		//mt_set_gpio_out(GPIO_BAT_ID,1);
#endif

	if (i2c_add_driver(&mt8193_i2c_driver)) {
		pr_err("%s: unable to add mt8193 i2c driver\n", __func__);
		return -1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mt8193_remove(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);
	i2c_del_driver(&mt8193_i2c_driver);
	return 0;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_MTK_MULTIBRIDGE_SUPPORT
static struct platform_device mtk_multibridge_dev = {
    .name = "multibridge",
    .id   = 0,
};
#endif


#ifdef CONFIG_OF
static const struct of_device_id multibridge_of_ids[] = {
	{.compatible = "mediatek,multibridge", },
	{}
};
#endif

static struct platform_driver mt8193_mb_driver = {
	.probe		= mt8193_probe,
	.remove		= mt8193_remove,
	.driver		= {
		.name	= "multibridge",
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = multibridge_of_ids,
#endif
	}
};

#define uint08 unsigned char 
extern int Setting_input_video(int video);
extern void Setting_Led_Current(uint08 led_drive_current);
extern int Setting_Image_Correct(uint08 m_throw,uint08 l_throw,uint08 m_DMD,uint08 l_DMD,uint08 l_PP,uint08 m_PP);
extern void dpp3438_power_on(void);
extern void dpp3438_power_off(void);
extern void write_dpp3430_i2c(uint08 addr, uint08 subaddr, uint08 *Param, uint08 ParamSize);
extern unsigned char key_switch_flag;
extern bool hdmi_open_flag;
extern wait_queue_head_t dpp3438_thread_wq;
extern int Seting_Image_Rotation(int val);
uint08 system_bootup_flag=0;
extern void dpp3438_logo_show(void);
extern int dpp3438_status_flag;
extern int dpp3438_power_rate ;
extern int reset_mic_cx20810(void);


static ssize_t hdmi_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
   char pbuf[50] ,**argv=pbuf;
   uint08 m_throw,l_throw,m_DMD,l_DMD,l_PP,m_PP;
   int video;
   if(copy_from_user(pbuf, buf,count))
   {
	   return	 -EFAULT;  
   }
   printk("111111%c,%d\n",pbuf[0],pbuf[1]);
   if(pbuf[0]=='A') //inter
   {
			   mt_set_gpio_mode(GPIO_JTCK_PIN, GPIO_MODE_00);  ///TEST
			   mt_set_gpio_dir(GPIO_JTCK_PIN, GPIO_DIR_OUT) ;	
			   mt_set_gpio_out(GPIO_JTCK_PIN,1);
			   
			   mt_set_gpio_mode(GPIO_JTDI_PIN, GPIO_MODE_00);  
			   mt_set_gpio_dir(GPIO_JTDI_PIN, GPIO_DIR_OUT) ;	
			   mt_set_gpio_out(GPIO_JTDI_PIN,0);
			   
			   mt_set_gpio_mode(GPIO_JTDO_PIN, GPIO_MODE_00);  
			   mt_set_gpio_dir(GPIO_JTDO_PIN, GPIO_DIR_OUT) ;	
			   mt_set_gpio_out(GPIO_JTDO_PIN,0);

			   dpp3438_status_flag &=0xfd;
   }
   else if(pbuf[0]=='B') //external
   {
			   mt_set_gpio_mode(GPIO_JTCK_PIN, GPIO_MODE_00);  ///TEST
			   mt_set_gpio_dir(GPIO_JTCK_PIN, GPIO_DIR_OUT) ;	
			   mt_set_gpio_out(GPIO_JTCK_PIN,1);			  
				   
			   mt_set_gpio_mode(GPIO_JTDI_PIN, GPIO_MODE_00);  
			   mt_set_gpio_dir(GPIO_JTDI_PIN, GPIO_DIR_OUT) ;	
			   mt_set_gpio_out(GPIO_JTDI_PIN,1);
			   
			   mt_set_gpio_mode(GPIO_JTDO_PIN, GPIO_MODE_00);  
			   mt_set_gpio_dir(GPIO_JTDO_PIN, GPIO_DIR_OUT) ;	
			   mt_set_gpio_out(GPIO_JTDO_PIN,1);
			   dpp3438_status_flag |=0x02;
   }
   else if(pbuf[0]=='C') //power on
   {
	   mt_set_gpio_mode(GPIO_HDMIPOWER_PIN, GPIO_MODE_00);	
	   mt_set_gpio_dir(GPIO_HDMIPOWER_PIN, GPIO_DIR_OUT) ;	 
	   mt_set_gpio_out(GPIO_HDMIPOWER_PIN,1);

   }
    else if(pbuf[0]=='D') //power off
    {
		//mt_set_gpio_mode(GPIO_HDMIPOWER_PIN, GPIO_MODE_00);  
		//mt_set_gpio_dir(GPIO_HDMIPOWER_PIN, GPIO_DIR_OUT) ;   
		mt_set_gpio_out(GPIO_HDMIPOWER_PIN,0);

   }
   else if(pbuf[0]=='E')
   {
   	if(pbuf[1]=='A')
	  Setting_Led_Current(1);
	else if(pbuf[1]=='B')
	Setting_Led_Current(2);
	else if(pbuf[1]=='C')
	Setting_Led_Current(3);
	else if(pbuf[1]=='D')
	  system_bootup_flag=true;
	else if(pbuf[1]=='E')
	  system_bootup_flag=false;	
	else if(pbuf[1]=='F')
	return reset_mic_cx20810();
   }
   else if(pbuf[0]=='F')
   {
         video =simple_strtol(&pbuf[1], NULL, 10);
   	Setting_input_video(video);
	printk("LLL=%d\n",video);
   }
    else if(pbuf[0]=='G')
    {

   	 l_throw =     simple_strtol(&pbuf[1], NULL, 10);
	  m_throw =  simple_strtol(&pbuf[5], NULL, 10);
	   l_DMD =     simple_strtol(&pbuf[9], NULL, 10);
	   m_DMD =   simple_strtol(&pbuf[13], NULL,10);
	   l_PP      =    simple_strtol(&pbuf[17], NULL,10);
	  m_PP      =    simple_strtol(&pbuf[21], NULL,10);
 	  Setting_Image_Correct(m_throw,l_throw,m_DMD,l_DMD,l_PP,m_PP);

	  printk("ddddd=%d,%d,%d,%d,%d,%d\n",l_throw,m_throw,l_DMD,m_DMD,l_PP,m_PP);
    }
  else if(pbuf[0]=='H')
	 {
	   uint08 regaddr,value,len=1;
	   uint08 buf[2];

	  value =     simple_strtol(&pbuf[1], NULL, 16);
	  Seting_Image_Rotation(value);
	 }
	 
   return count;
}

static ssize_t hdmi_read( struct file *file,char __user *buffer, size_t len, loff_t *offset )
{
	return len;
}
static int hdmi_release (struct inode *node, struct file *file)
{

	return 0;

}
static int hdmi_open (struct inode *inode, struct file *file)
{
	
	return nonseekable_open(inode, file);
}

static long hdmi_unlocked_ioctl (struct file *pfile, unsigned int cmd, unsigned long param)
{
	struct ddp_config ddp3438;
	 struct stm_config stm_ctl;
	int err;
	int charger_type;
	void __user *argp = (void __user *)param;
		switch(cmd)
		{
		  case DPP3438_POWER_ON:
			if(hdmi_open_flag == true)
			{
			key_switch_flag=1;
			wake_up(&dpp3438_thread_wq);			
			}
			break;
		case DPP3438_POWER_OFF:
			if(hdmi_open_flag == false)
			{
			key_switch_flag=1;
			wake_up(&dpp3438_thread_wq);
			}
			
			break;
		case DPP3438_CORRECT:
			if(copy_from_user(&ddp3438, (struct ddp_config*)param, sizeof(ddp3438)))
			{
				err = -EFAULT;
				goto err_out;
			}
			Setting_Image_Correct(ddp3438.m_throw,ddp3438.l_throw,ddp3438.m_DMD,ddp3438.l_DMD,ddp3438.l_PP,ddp3438.m_PP);
			printk("ddp3438.l_throw=%d,  %d,,,%d,,,%d\n",ddp3438.m_throw,ddp3438.l_throw,ddp3438.m_DMD,ddp3438.l_DMD);
			break;
		case DPP3438_POWER_RATE:
			if(copy_from_user(&ddp3438, (void __user*)param, sizeof(ddp3438)))
			{
				err = -EFAULT;
				goto err_out;
			}
			Setting_Led_Current(ddp3438.power_rate);
			dpp3438_power_rate=ddp3438.power_rate;
			break;
		case STM_POWER_CTL:
			if(copy_from_user(&stm_ctl, (void __user*)param, sizeof(stm_ctl)))
			{
				err = -EFAULT;
				goto err_out;
			}
			if(stm_ctl.pin ==101)
			{
			mt_set_gpio_mode(GPIO_101, GPIO_MODE_00);	
			mt_set_gpio_dir(GPIO_101, GPIO_DIR_OUT) ;	 
			mt_set_gpio_out(GPIO_101,stm_ctl.val);
			}
			else if(stm_ctl.pin ==102)
			{
				mt_set_gpio_mode(GPIO_102, GPIO_MODE_00);	
				mt_set_gpio_dir(GPIO_102, GPIO_DIR_OUT) ;	 
				mt_set_gpio_out(GPIO_102,stm_ctl.val);

			}
			else if(stm_ctl.pin ==119)
			{
				mt_set_gpio_mode(GPIO_119, GPIO_MODE_00);	
				mt_set_gpio_dir(GPIO_119, GPIO_DIR_OUT) ;	 
				mt_set_gpio_out(GPIO_119,stm_ctl.val);

			}
			
			break;

			case DPP3438_STATUS:
				if(copy_to_user(argp, &dpp3438_status_flag, sizeof(dpp3438_status_flag)))
				{
					err = -EFAULT;
					goto err_out;
				}			   
			break;
			case CHECK_CHARGER_TYPE:
				charger_type= mt_get_gpio_in(CHG_DET_PIN);
				if(copy_to_user(argp, &charger_type, sizeof(charger_type)))
				{
					err = -EFAULT;
					goto err_out;
				}			   
			break;

		}

 err_out:
	 return err;
}

static struct file_operations hdmi_fops = {
	.owner = THIS_MODULE,
	.open = hdmi_open,
	.release = hdmi_release,
	.write	= hdmi_write,
	.read		= hdmi_read,
	.unlocked_ioctl = hdmi_unlocked_ioctl,
};

/*----------------------------------------------------------------------------*/
static void __exit mt8193_mb_exit(void)
{
	platform_driver_unregister(&mt8193_mb_driver);
#if 1
	if(hdmi_cdev != NULL)
          {
	        cdev_del(hdmi_cdev);
	        hdmi_cdev = NULL;
         }
#endif
         unregister_chrdev_region(m_dev, 1);
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int __init mt8193_mb_init(void)
{
	int ret = 0;

	pr_info("%s\n", __func__);
	 mt_set_gpio_mode(GPIO_BAT_ID, GPIO_MODE_00);  //hdmi ic(mt8193 3.3V 1.2V)  power
		mt_set_gpio_dir(GPIO_BAT_ID, GPIO_DIR_OUT) ;   
		mt_set_gpio_out(GPIO_BAT_ID,0);
#ifdef CONFIG_MTK_MULTIBRIDGE_SUPPORT
    ret = platform_device_register(&mtk_multibridge_dev);
    printk("[%s]: multibridge_driver_device, retval=%d \n!", __func__, ret);
    if (ret != 0){
	printk("failed to register mt8193_mb_driver, ret=%d\n", ret);
        return ret;
    }
#endif
	/*i2c_register_board_info(1, &i2c_mt8193, 1);*/
	ret = platform_driver_register(&mt8193_mb_driver);
	if (ret) {
		printk("failed to register mt8193_mb_driver, ret=%d\n", ret);
		return ret;
	}
#if 1
		int err;
		  ret = alloc_chrdev_region(&m_dev, 0, 1, DEV_NAME);
				 if (ret< 0) {
				 printk("motor	alloc_chrdev_region failed, %d", ret);
				return ret;
			}
			 hdmi_cdev= cdev_alloc();
			 if (hdmi_cdev == NULL) {
					 printk("motor cdev_alloc failed");
					 ret = -ENOMEM;
					 goto EXIT;
				 }
			cdev_init(hdmi_cdev, &hdmi_fops);
			 hdmi_cdev->owner = THIS_MODULE;
			 ret = cdev_add(hdmi_cdev, m_dev, 1);
			 if (ret < 0) {
				  printk("Attatch file motor operation failed, %d", ret);
				 goto EXIT;
			 }
		 hdmi_class = class_create(THIS_MODULE, DEV_NAME);
				 if (IS_ERR(hdmi_class)) {
					 printk("Failed to create class(motor)!\n");
					 return PTR_ERR(hdmi_class);
				 }
				 
		 h_device = device_create(hdmi_class, NULL, m_dev, NULL,DEV_NAME);
		 if (IS_ERR(h_device))
			 printk("Failed to create motor_dev device\n");
#endif
	mt_set_gpio_out(GPIO_BAT_ID,1);

	return 0;
EXIT:
	 if(hdmi_cdev != NULL)
          {
	        cdev_del(hdmi_cdev);
	        hdmi_cdev = NULL;
         }

         unregister_chrdev_region(m_dev, 1);
		  
	return ret;

}

static int __init mt8193_i2c_board_init(void)
{
	int ret = 0;
	pr_info("%s\n", __func__);
	ret = i2c_register_board_info(MT8193_I2C_ID, &i2c_mt8193, 1);
	if (ret)
		pr_err("failed to register mt8193 i2c_board_info, ret=%d\n", ret);
	return ret;	
}
/*----------------------------------------------------------------------------*/
core_initcall(mt8193_i2c_board_init);
module_init(mt8193_mb_init);
module_exit(mt8193_mb_exit);
MODULE_AUTHOR("SS, Wu <ss.wu@mediatek.com>");
MODULE_DESCRIPTION("MT8193 Driver");
MODULE_LICENSE("GPL");

#endif
