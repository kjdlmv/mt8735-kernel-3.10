#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/module.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#define GP_CLASS_NAME  "i2c_operator"
#define GP_CHR_DEV_NAME "i2c_operator"
#define GP_I2C_DEV_NAME "i2c_operator_device"

//static unsigned int map_io_base;
static void __iomem *map_io_base;

#define D_IO_BASE   map_io_base
#define D_IO_BASE_PHY  (0x11240000) //(0x01c20800)
#define D_IO_LEN    (0x400)
#define D_IO_B  (1)
#define D_IO_C  (2)
#define D_IO_D  (3)
#define D_IO_E  (4)
#define D_IO_F  (5)
#define D_IO_G  (6)
#define D_IO_H  (7)
#define D_IO_CFG0(x)    (D_IO_BASE+(x*0x24)+0x00)
#define D_IO_CFG1(x)    (D_IO_BASE+(x*0x24)+0x04)
#define D_IO_CFG2(x)    (D_IO_BASE+(x*0x24)+0x08)
#define D_IO_CFG3(x)    (D_IO_BASE+(x*0x24)+0x0c)
#define D_IO_DAT(x)     (D_IO_BASE+(x*0x24)+0x10)
#define D_IO_DRV0(x)    (D_IO_BASE+(x*0x24)+0x14)
#define D_IO_DRV1(x)    (D_IO_BASE+(x*0x24)+0x18)
#define D_IO_PUL0(x)    (D_IO_BASE+(x*0x24)+0x1c)
#define D_IO_PUL1(x)    (D_IO_BASE+(x*0x24)+0x20)


static struct class * p_gp_class;
static struct device * p_class_i2c;
static int major_num;
static unsigned char * i2c_msg;


int send_to_device(char * data, int count);

int receive_from_device(char * data, int count);
int cx20810_set_mode(int mode, int index);
//int led_array_set_mode(unsigned char * param, int length);
int led_array_set_enable(int enable);

enum
{
	I2C_DEVICE_FPGA = 0,
	I2C_DEVICE_CX20810,
	I2C_DEVICE_CX20810_0,
	I2C_DEVICE_CX20810_1,
	I2C_DEVICE_CX20810_2,
	I2C_DEVICE_LED_ARRAY,
};
#if 0
static void gp_hw_init(void)
{
	printk("i2c_operator.c->gp_hw_init()\n");

	if(!D_IO_BASE)
	{
		printk("Timothy:D_IO_BASE is not initial yet, initial it now\n");
		//D_IO_BASE = (unsigned int)ioremap(D_IO_BASE_PHY, D_IO_LEN);
		D_IO_BASE = ioremap(D_IO_BASE_PHY, D_IO_LEN);
		if(!D_IO_BASE)
		{
			printk("led array hardware init io error\n");
			return -1;
		}
	}
	
	// set pe6 output and set it high to open mute led
	writel(readl(D_IO_CFG0(D_IO_E)) & (~(0x7<<24)) | (0x1<<24), D_IO_CFG0(D_IO_E));
	writel(readl(D_IO_DAT(D_IO_E)) | 0x1<<6, D_IO_DAT(D_IO_E));
}
#endif

// add by Timothy
// time:2015-02-09
// start==========
// i2c设备write处理
static ssize_t gp_i2c_write_dispatch(int len)
{
//	printk("Timothy:i2c_operator.c->gp_i2c_write_dispatch(), len = %d\n", len);
	int ret = 0;
	int i;
	int device_index = (int)i2c_msg[0];
	int length = len-sizeof(char);
	printk("Timothy:device_index = %d\n", device_index);

	//all 3 cx20810
	if(I2C_DEVICE_CX20810 == device_index)
	{
//		printk("Timothy:operate device:cx20810\n");
		for(i = 0; i < 3; i++)
		{
			cx20810_set_mode((int)i2c_msg[1], i);
		}
		return 0;
	}

	else if(I2C_DEVICE_CX20810_0 == device_index)
	{
//		printk("Timothy:operate device:cx20810_0\n");
		return cx20810_set_mode((int)i2c_msg[1], 0);
	}
	else if(I2C_DEVICE_CX20810_1 == device_index)
	{
//		printk("Timothy:operate device:cx20810_1\n");
		return cx20810_set_mode((int)i2c_msg[1], 1);
	}
	else if(I2C_DEVICE_CX20810_2 == device_index)
	{
//		printk("Timothy:operate device:cx20810_2\n");
		return cx20810_set_mode((int)i2c_msg[1], 2);
	}

	// LED
//	else if(I2C_DEVICE_LED_ARRAY == device_index)
//	{
//		printk("Timothy:operate device:led_array\n");
//		return led_array_set_mode(&i2c_msg[1], length);
//	}

	// FPGA
	else if(I2C_DEVICE_FPGA == device_index)
	{
//		printk("Timothy:operate device:FPGA, data lendth is %d\n", length);
		ret = send_to_device((char *)&i2c_msg[1], length);
		return ret;
	}
	return -1;
}

static int gp_i2c_read_dispatch(int len)
{
//	printk("Timothy:i2c_operator.c->gp_i2c_read_dispatch()\n");
	int ret =  0;
	int device_index = (int)i2c_msg[0];
	int length = len-sizeof(char);
//	printk("Timothy:device_index = %d\n", device_index);
	if(I2C_DEVICE_FPGA == device_index) // FPGA
	{
//		printk("Timothy:operate device:FPGA, data lendth is %d\n", length);
		ret = receive_from_device((char *)&i2c_msg[1], length);
		return ret;
	}
	return -1;
}
// end==========
extern void init_i2c_io(void);

static int gp_open(struct inode * pnode, struct file * pfile)
{
	printk("Timothy:i2c_operator.c->gp_open()\n");
	int major = MAJOR(pnode->i_rdev);
	int minor = MINOR(pnode->i_rdev);

	init_i2c_io();

	if(minor == 0)
	{
		pfile->private_data = (void*)p_class_i2c;
		printk("Timothy:gp_open_i2c\n");
	}
	else
	{
		pfile->private_data = (void*)NULL;
		printk("Timothy:gp_open:unknow device\n");
	}
	return 0;
}

static int gp_close(struct inode * pnode, struct file * pfile)
{
	pfile->private_data = (void*)NULL;
	printk("gp_close\n");
	return 0;
}

static ssize_t gp_read(struct file * pfile, char __user * puser, size_t len, loff_t * poff)
{
	int nread = 0;
	int i;
//	printk("Timothy:i2c_operator.c->gp_read()\n");
	if(pfile->private_data == p_class_i2c)
	{
//		printk("Timothy:gp_read():i2c device\n");
		i2c_msg = (unsigned char*)kzalloc(len * sizeof(unsigned char), GFP_KERNEL);
		copy_from_user((void*)i2c_msg, puser, len);
		nread = gp_i2c_read_dispatch(len);
//		printk("Timothy:the data will send to user is:%d\n", i2c_msg[1]);
		copy_to_user(puser, (void*)i2c_msg, len);
		kfree(i2c_msg);
	}
//	printk("Timothy:return value is %d\n", nread);
	return nread;
}

static ssize_t gp_write(struct file * pfile, const char __user * puser, size_t len, loff_t * poff)
{
//	printk("Timothy:i2c_operator.c->gp_write()\n");
	int ret = 0;
	int i;
	if(pfile->private_data == p_class_i2c)
	{
//		printk("Timothy:gp_write():i2c device\n");
		i2c_msg = (unsigned char *)kzalloc(len*sizeof(unsigned char), GFP_KERNEL);
		copy_from_user((void*)i2c_msg, puser, len);

		printk("Timothy:the data is ");
		for(i = 0; i < len; i++)
		{
			printk("%x ", i2c_msg[i]);
		}
		printk("\n");
		ret = gp_i2c_write_dispatch(len);
		kfree(i2c_msg);
	}
	else
	{
//		printk("Timothy:gp_write_null\n");
	}
//	printk("Timothy:return value is %d\n", ret);
	return ret;
}

static struct file_operations gp_fops =
{
	.owner = THIS_MODULE,
	.open = gp_open,
	.release = gp_close,
	.read = gp_read,
	.write = gp_write,
};

static int __init i2c_operator_init(void)
{
	//gp_hw_init();

	major_num = register_chrdev(0, GP_CHR_DEV_NAME, &gp_fops);
	if (major_num < 0)
	{
		printk("Timothy:gen_prov_reg_chr_err\n");
		return 1;
	}
	p_gp_class = class_create(THIS_MODULE, GP_CLASS_NAME);
	if(p_gp_class == NULL)
	{
		printk("Timothy:gen_prov:class create err\n");
	}

	p_class_i2c = device_create(p_gp_class, NULL, MKDEV(major_num, 0), NULL, GP_I2C_DEV_NAME);
	if(p_class_i2c == NULL)
	{
		printk("Timothy:gen_prov:device_create err:i2c\n");
		return 1;
	}
}

static void __exit i2c_operator_exit(void)
{
}

module_init(i2c_operator_init);
module_exit(i2c_operator_exit);

MODULE_AUTHOR("Timothy");
MODULE_DESCRIPTION("i2c operator driver");
MODULE_LICENSE("GPL");
