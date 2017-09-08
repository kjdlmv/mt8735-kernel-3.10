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
#include <linux/i2c.h>
#include <mach/mt_gpio.h>
#include <mach/mt_typedefs.h>

#include <i2c_wrapper.h>

kal_bool wrapper_i2c_write_reg_org(struct i2c_client *client, u8 regdata[],u8 length)
{
	kal_bool ack = KAL_TRUE;
	unsigned char ret;
	unsigned char wrbuf[length];
	int i = 0;

	
	for (i=0; i<length; i++)
	{
		wrbuf[i] = regdata[i];
	}
	
	ret = i2c_master_send(client, wrbuf, length);
	if (ret != length) {
		I2C_WRP_ERR("%s: failed ret = %d\n", client->name, ret);
		ack = KAL_FALSE;
	}

	return ack;
}

kal_bool wrapper_i2c_write_regdata(struct i2c_client *client, u8 reg,u8 data)
{
	kal_bool ack = KAL_TRUE;
	unsigned char i;
	unsigned char wrbuf[2];

	wrbuf[0] = reg;
	wrbuf[1] = data;
	
	for (i=0; i<I2C_MAX_TRY; i++)
	{
		ack = wrapper_i2c_write_reg_org(client, wrbuf, 2);
		if (ack == KAL_TRUE) // ack success
			break;
	}
	return ack;
}

kal_bool wrapper_i2c_write_data(struct i2c_client *client, u8 data[], u8 length)
{
	kal_bool ack = KAL_FALSE;
	unsigned char i;

	for (i=0; i<I2C_MAX_TRY; i++)
	{
		ack = wrapper_i2c_write_reg_org(client, data, length);
		if (ack == KAL_TRUE) // ack success
			break;
	}
	return ack;
}



kal_bool wrapper_i2c_write_regs(struct i2c_client *client, I2C_REGDATA_T regsdata[],u8 length)
{
	int i = 0;
	for (i = 0; i<length; i++)
	{
		if(wrapper_i2c_write_regdata(client,regsdata[i].reg, regsdata[i].data) == KAL_FALSE)
		{
			I2C_WRP_ERR("when write regsdata[%d] failed\n", i);
			return KAL_FALSE;
		}
		
	}

	return KAL_TRUE;
}

kal_bool wrapper_i2c_read_regdata(struct i2c_client *client, u8 regaddr, u8 *data, u8 length)
{
	unsigned char rdbuf[1], wrbuf[1], ret, i;

	wrbuf[0] = regaddr;
	kal_bool ack = KAL_FALSE;

	for (i=0; i<I2C_MAX_TRY; i++) 
	{
		ret = i2c_master_send(client, wrbuf, 1);
		
		if (ret == 1){
			break;
		}
		else if(ret != 1 && i == I2C_MAX_TRY)
		{
			I2C_WRP_ERR("%s: failed\n", client->name);
			ack = KAL_FALSE;
			return ack;
		}
	}
	
	for (i=0; i<I2C_MAX_TRY; i++) 
	{
		ret = i2c_master_recv(client, data, length);

		if (ret != length)
		{
			I2C_WRP_ERR("%s: failed\n", client->name);
			ack = KAL_FALSE;
		}
	}
	ack = KAL_TRUE;
	
    return ack;
		
}

kal_bool wrapper_i2c_read_data(struct i2c_client *client, u8 *data, u8 length) 
{
	kal_bool ack = KAL_TRUE;
	unsigned char ret;
	unsigned char i = 0;
	
	for (i=0; i<I2C_MAX_TRY; i++)
	{
		ret = i2c_master_recv(client, data, length);
	}
	
	if (ret != length)
	{
		I2C_WRP_ERR("%s: failed\n", client->name);
		ack = KAL_FALSE;
	}
	
    return ack;
}
//=========================================================================
void delay_nop_1us(u16 wTime)
{
    u16 i, j;
#if defined(MT6223) 
    for (i=0; i<wTime*30; i++) ;  
#elif defined(MT6235)
    for (i=0; i<wTime*8; i++) ;
#else
    for (i=0; i<wTime*15; i++) ;
#endif
}

#define SDA_SET_OUT(sda)		mt_set_gpio_dir(sda, GPIO_DIR_OUT)
#define SDA_SET_IN(sda)			mt_set_gpio_dir(sda, GPIO_DIR_IN)
								

#define SDA_OUT_H(sda)		mt_set_gpio_out(sda, GPIO_OUT_ONE)
#define SDA_OUT_L(sda)		mt_set_gpio_out(sda, GPIO_OUT_ZERO)	
#define SDA_GET_IN(sda)		mt_get_gpio_in(sda)

#define SCL_OUT_H(scl)		mt_set_gpio_out(scl, GPIO_OUT_ONE)
#define SCL_OUT_L(scl)		mt_set_gpio_out(scl, GPIO_OUT_ZERO)

#define NOP 	10
#define NOP2 	10*2

void i2c_init_gpio(I2C_GPIO_T *dev)
{
	mt_set_gpio_mode(dev->scl, GPIO_MODE_00);
	mt_set_gpio_dir(dev->scl, GPIO_DIR_OUT);
	mt_set_gpio_pull_enable(dev->scl, TRUE);
	mt_set_gpio_out(dev->scl, GPIO_OUT_ONE);

	mt_set_gpio_mode(dev->sda, GPIO_MODE_00);
	mt_set_gpio_dir(dev->sda, GPIO_DIR_OUT);
	mt_set_gpio_pull_enable(dev->scl, TRUE);
	mt_set_gpio_out(dev->sda, GPIO_OUT_ONE);
}
void i2c_start(I2C_GPIO_T *dev)
{
	u32 sda = dev->sda;
	u32 scl = dev->scl;

	SDA_OUT_H(sda);
	SCL_OUT_H(scl);
	delay_nop_1us(NOP);

	SDA_OUT_L(sda);
	delay_nop_1us(NOP2);
	//SDA_OUT_H(sda);
	//delay_nop_1us(10);
}

void i2c_stop(I2C_GPIO_T *dev)
{
	u32 sda = dev->sda;
	u32 scl = dev->scl;

	SCL_OUT_L(scl);
	delay_nop_1us(NOP);
	
	SDA_OUT_L(sda);
	delay_nop_1us(NOP);

	SCL_OUT_H(scl);
	delay_nop_1us(NOP);
	
	SDA_OUT_H(sda);
	delay_nop_1us(NOP);
}


kal_bool i2c_transfer_byte(I2C_GPIO_T *dev, u8 data)
{
	u32 sda = dev->sda;
	u32 scl = dev->scl;
	u8 tmp = data;
	kal_bool ack = KAL_FALSE;
	u8 i, index;
	for(i = 0; i < 8; i++)
	{
		
		SCL_OUT_L(scl);
		delay_nop_1us(NOP);
		if(tmp & 0x80)
		{
			SDA_OUT_H(sda);
			delay_nop_1us(NOP);
		}
		else
		{
			SDA_OUT_L(sda);
			delay_nop_1us(NOP);
		}
		SCL_OUT_H(scl);
		delay_nop_1us(NOP2);
		tmp = tmp << 1;
	}

	SCL_OUT_L(scl);
	delay_nop_1us(NOP);

	SDA_SET_IN(sda);
	delay_nop_1us(NOP);
	
	SCL_OUT_H(scl);
	delay_nop_1us(NOP);
	if(SDA_GET_IN(sda))
	{
		ack = KAL_FALSE;
	}
	else
	{
		ack = KAL_TRUE;

	}
	SCL_OUT_L(scl);
	delay_nop_1us(NOP);

	SDA_SET_OUT(sda);

	return ack;
}
u8 i2c_transfer_bytes(I2C_GPIO_T *dev, u8 *data, u8 length)
{
	u8 i = 0;
	for(i = 0; i < length; i++)
	{
		if(i2c_transfer_byte(dev, data[i]) == KAL_FALSE)
			return i;
	}
	
	return length;
	
}
kal_bool i2c_write_reg_org(I2C_GPIO_T *dev, u8 regdata[],u8 length)
{
	kal_bool ack = KAL_TRUE;
	unsigned char ret;
	unsigned char wrbuf[length+1];
	int i = 0;

	i2c_start(dev);

	wrbuf[0] = dev->addr;
	
	for (i = 0; i<length; i++)
	{
		wrbuf[i+1] = regdata[i];
	}
	
	ret = i2c_transfer_bytes(dev, wrbuf, length+1);
	if (ret != (length + 1)) {
		I2C_WRP_ERR(" failed\n");
		ack = KAL_FALSE;
	}
	
	i2c_stop(dev);

	return ack;
}
kal_bool i2c_write_regdata(I2C_GPIO_T *dev, u8 reg,u8 data)
{
	kal_bool ack = KAL_TRUE;
	unsigned char i;
	unsigned char wrbuf[2];

	wrbuf[0] = reg;
	wrbuf[1] = data;
	
	for (i=0; i<I2C_MAX_TRY; i++)
	{
		ack = i2c_write_reg_org(dev, wrbuf, 2);
		if (ack == KAL_TRUE) // ack success
			break;
	}
	return ack;
}

kal_bool i2c_write_regs(I2C_GPIO_T *dev, I2C_REGDATA_T regsdata[],u8 length)
{
	int i = 0;
	for (i = 0; i<length; i++)
	{
		if(i2c_write_reg_org(dev,(u8 *)&regsdata[i].reg, 2) == KAL_FALSE)
		{
			I2C_WRP_ERR("when write regsdata[%d] failed\n", i);
			return KAL_FALSE;
		}
		
	}

	return KAL_TRUE;
}

