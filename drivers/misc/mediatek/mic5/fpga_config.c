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
#include <linux/sysfs.h>
//#include <linux/stdlib.h>
#include <linux/string.h>
#include <linux/stat.h>

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>

//unsigned char toupad_get_key(void);
//void tipa_set_vol(unsigned char abs_vol);
int at_8740_write(unsigned char * pbuf, unsigned int len);
int at_8740_read(unsigned char * pbuf, unsigned int len);
void at_8740_wakeup(void);

//static unsigned int map_io_base;
static void __iomem *map_io_base;
#define D_IO_BASE   map_io_base
#define D_IO_BASE_PHY   (0x11240000)//(0x01c20800)
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

#define GPIO_TCK_PIN   (GPIO97|0x80000000)
#define GPIO_TMS_PIN   (GPIO98|0x80000000)
#define GPIO_JTAGEN_PIN   (GPIO100|0x80000000)
#define GPIO_RST_PIN   (GPIO101|0x80000000)



int cx20810_set_mode(int mode, int index);

typedef struct
{
    unsigned int opcode;
    unsigned int param[3];
} t_data_unit;

enum
{
    eDU_RESVER = 0,
    eDU_IVW_PRESET_TIME,
    eDU_REC_TYPE,
    eDU_IVW_POS_INFO,
    eDU_MAX
};

t_data_unit database[eDU_MAX];

static int major_num;
static struct class * p_class;
static struct device * p_fpga;
static struct device * p_reg_debug;
static struct device * p_dsp_settings;
static struct device * p_au;
static struct device * p_tk;
static struct device * p_vol;
static struct device * p_vol2;
static struct device * p_8740;
static struct device * p_8740_wakeup;

static unsigned int param_config;

static struct kset * kset_vendor;
static struct kobject kobj_pa;

static void gpio_init(void)
{
	mt_set_gpio_mode(GPIO_TCK_PIN, GPIO_MODE_00);  
	mt_set_gpio_dir(GPIO_TCK_PIN, GPIO_DIR_OUT) ; 

	mt_set_gpio_mode(GPIO_TMS_PIN, GPIO_MODE_00);  
	mt_set_gpio_dir(GPIO_TMS_PIN, GPIO_DIR_OUT) ; 

	mt_set_gpio_mode(GPIO_JTAGEN_PIN, GPIO_MODE_00);  
	mt_set_gpio_dir(GPIO_JTAGEN_PIN, GPIO_DIR_OUT) ; 

	mt_set_gpio_mode(GPIO_RST_PIN, GPIO_MODE_00);  
	mt_set_gpio_dir(GPIO_RST_PIN, GPIO_DIR_OUT) ; 

}
static ssize_t gen_attr_show(struct kobject *kobj, struct attribute *attr,
                             char *buf)
{
    strcpy(buf, "wocao");
    return strlen(buf);
}

static ssize_t gen_attr_store(struct kobject *kobj, struct attribute *attr,
                              const char *buf, size_t count)
{
    int i = simple_strtol(buf, buf + strlen(buf) - 1, 0);
    printk("vol=%ddb", i);
   // tipa_set_vol(2 * (24-i));
    return count;
}

struct attribute gen_vol_set =
{
    .name = "vol_set",
    .mode = S_IRWXUGO,
};

struct attribute * gen_attr_group[] =
{
    &gen_vol_set,
    NULL,
};

static const struct sysfs_ops gen_ops =
{
    .show      = gen_attr_show,
    .store     = gen_attr_store,
};

static void gen_release(struct kobject * k)
{
//  nothing to do
}

static struct kobj_type gen_ktype =
{
    .sysfs_ops  = &gen_ops,
    .default_attrs  = gen_attr_group,
    .release = gen_release,
};

static int gp_open(struct inode * pnode, struct file * pfile)
{
    int major = MAJOR(pnode->i_rdev);
    int minor = MINOR(pnode->i_rdev);
 printk("openfpga--minor=%d\n",minor);
    if(minor == 0)
    {
        pfile->private_data = (void*)p_fpga;
    }
    else if(minor == 1)
    {
        pfile->private_data = (void*)p_reg_debug;
    }
    else if(minor == 2)
    {
        pfile->private_data = (void*)p_dsp_settings;
    }
    else if(minor == 3)
    {
        pfile->private_data = (void*)p_au;
    }
    else if(minor == 4)
    {
        pfile->private_data = (void*)p_tk;
    }
    else if(minor == 5)
    {
        pfile->private_data = (void*)p_vol;
    }
//    else if(minor == 6)
//    {
//        pfile->private_data = (void*)p_8740;
//    }
//    else if(minor == 7)
//    {
//        pfile->private_data = (void*)p_8740_wakeup;
//    }
    else
    {
        pfile->private_data = NULL;
    }

    return 0;
}

static int gp_close(struct inode * pnode, struct file * pfile)
{
    return 0;
}

static ssize_t gp_read(struct file * pfile, char __user * puser, size_t len, loff_t * poff)
{
    if(pfile->private_data == p_fpga)
    {
        return 0;
    }
    else if(pfile->private_data == p_reg_debug)
    {
    	#if 0
	        unsigned int base_addr = (unsigned int)ioremap(0x1C22400, 0x100);
	        unsigned int i;
	        for(i = 0; i <= 0x3C; i += 4)
	        {
	            printk("[i2s1] addr:0x%x val:0x%x\r\n", base_addr + i, *((unsigned int *)(base_addr + i)));
	        }
	#endif
        return len;
    }
    else if(pfile->private_data == p_au)
    {
        t_data_unit tmp_data;
        copy_from_user((void*)&tmp_data, puser, len);
        if(tmp_data.opcode < eDU_MAX)
        {
            copy_to_user(puser, &database[tmp_data.opcode], sizeof(t_data_unit));
            return len;
        }
        else
        {
            return 0;
        }
    }
//    else if(pfile->private_data == p_tk)
//    {
//        unsigned char tmp = toupad_get_key();
//        copy_to_user(puser, &tmp, sizeof(unsigned char));
//        return len;
//    }
//    else if(pfile->private_data == p_8740)
//    {
//        unsigned char * pbuf;
//        pbuf = kmalloc(len, GFP_KERNEL);
//        printk("8740 nread:%d\r\n", at_8740_read(pbuf, len));
//        copy_to_user(puser, pbuf, len);
//        kfree(pbuf);
//        return len;
//    }
//    else if(pfile->private_data == p_8740_wakeup)
//    {
//        return len;
//    }
    return 0;
}

enum
{
    CX20810_NORMAL_MODE = 0,
    CX20810_NORMAL_MODE_SIMPLE,
    CX20810_NIRMAL_MODE_CODEC3,
    CX20810_NIRMAL_MODE_CODEC3_SIMPLE,
    CX20810_96K_16BIT_MODE,
    CX20810_48K_16BIT_MODE,
};
void fpga_init_gpio(void)
{
	gpio_init();
	mt_set_gpio_out(GPIO_JTAGEN_PIN, 0);
	mt_set_gpio_out(GPIO_TCK_PIN, 0);
	mt_set_gpio_out(GPIO_TMS_PIN, 0);
}
static ssize_t gp_write(struct file * pfile, const char __user * puser, size_t len, loff_t * poff)
{
    if(pfile->private_data == p_fpga)
    {
        copy_from_user((void*)&param_config, puser, len);
        switch(param_config)
        {
            case 0:
                printk("fpga_config:mode0\r\n");
   //             cx20810_set_mode(CX20810_NORMAL_MODE, 0);
  //              cx20810_set_mode(CX20810_NORMAL_MODE, 1);
 //               cx20810_set_mode(CX20810_NORMAL_MODE, 2);
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x7000000) | 0x1000000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) & ~(1 << 6)), D_IO_DAT(D_IO_D));
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x7000) | 0x1000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) & ~(1 << 3)), D_IO_DAT(D_IO_D));
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x70000) | 0x10000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) & ~(1 << 4)), D_IO_DAT(D_IO_D));
#if 0
                writel((readl(D_IO_CFG0(D_IO_E)) & ~0x70000) | 0x10000, D_IO_CFG0(D_IO_E));
                writel((readl(D_IO_DAT(D_IO_E)) & ~(1 << 4)), D_IO_DAT(D_IO_E));
                writel((readl(D_IO_CFG0(D_IO_H)) & ~0x70000000) | 0x10000000, D_IO_CFG0(D_IO_H));
                writel((readl(D_IO_DAT(D_IO_H)) & ~(1 << 7)), D_IO_DAT(D_IO_H));
                writel((readl(D_IO_CFG0(D_IO_E)) & ~0x7000) | 0x1000, D_IO_CFG0(D_IO_E));
                writel((readl(D_IO_DAT(D_IO_E)) & ~(1 << 3)), D_IO_DAT(D_IO_E));
#endif
		gpio_init();
		mt_set_gpio_out(GPIO_JTAGEN_PIN, 0);
		mt_set_gpio_out(GPIO_TCK_PIN, 0);
		mt_set_gpio_out(GPIO_TMS_PIN, 0);

                break;
            case 1:
                printk("fpga_config:mode1\r\n");
                cx20810_set_mode(CX20810_96K_16BIT_MODE, 1);
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x7000000) | 0x1000000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) & ~(1 << 6)), D_IO_DAT(D_IO_D));
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x7000) | 0x1000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) | (1 << 3)), D_IO_DAT(D_IO_D));
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x70000) | 0x10000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) & ~(1 << 4)), D_IO_DAT(D_IO_D));
#if 0
	      writel((readl(D_IO_CFG0(D_IO_E)) & ~0x70000) | 0x10000, D_IO_CFG0(D_IO_E));
                writel((readl(D_IO_DAT(D_IO_E)) & ~(1 << 4)), D_IO_DAT(D_IO_E));
                writel((readl(D_IO_CFG0(D_IO_H)) & ~0x70000000) | 0x10000000, D_IO_CFG0(D_IO_H));
                writel((readl(D_IO_DAT(D_IO_H)) & ~(1 << 7)), D_IO_DAT(D_IO_H));
                writel((readl(D_IO_CFG0(D_IO_E)) & ~0x7000) | 0x1000, D_IO_CFG0(D_IO_E));
                writel((readl(D_IO_DAT(D_IO_E)) | (1 << 3)), D_IO_DAT(D_IO_E));
#endif	
		gpio_init();
		mt_set_gpio_out(GPIO_JTAGEN_PIN, 0);
		mt_set_gpio_out(GPIO_TCK_PIN, 0);
		mt_set_gpio_out(GPIO_TMS_PIN, 1);

                break;
            case 2:
                printk("fpga_config:mode2\r\n");
                cx20810_set_mode(CX20810_48K_16BIT_MODE, 0);
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x7000000) | 0x1000000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) & ~(1 << 6)), D_IO_DAT(D_IO_D));
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x7000) | 0x1000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) & ~(1 << 3)), D_IO_DAT(D_IO_D));
//                writel((readl(D_IO_CFG0(D_IO_D)) & ~0x70000) | 0x10000, D_IO_CFG0(D_IO_D));
//                writel((readl(D_IO_DAT(D_IO_D)) | (1 << 4)), D_IO_DAT(D_IO_D));
#if 0
	writel((readl(D_IO_CFG0(D_IO_E)) & ~0x70000) | 0x10000, D_IO_CFG0(D_IO_E));
                writel((readl(D_IO_DAT(D_IO_E)) & ~(1 << 4)), D_IO_DAT(D_IO_E));
                writel((readl(D_IO_CFG0(D_IO_H)) & ~0x70000000) | 0x10000000, D_IO_CFG0(D_IO_H));
                writel((readl(D_IO_DAT(D_IO_H))  | (1 << 7)), D_IO_DAT(D_IO_H));
                writel((readl(D_IO_CFG0(D_IO_E)) & ~0x7000) | 0x1000, D_IO_CFG0(D_IO_E));
                writel((readl(D_IO_DAT(D_IO_E)) & ~(1 << 3)), D_IO_DAT(D_IO_E));
#endif
		gpio_init();
		mt_set_gpio_out(GPIO_JTAGEN_PIN, 0);
		mt_set_gpio_out(GPIO_TCK_PIN, 1);
		mt_set_gpio_out(GPIO_TMS_PIN, 0);

                break;
            default:
                break;
        }
        return len;
    }
    else if(pfile->private_data == p_reg_debug)
    {
        return len;
    }
    else if(pfile->private_data == p_dsp_settings)
    {
        return len;
    }
    else if(pfile->private_data == p_au)
    {
        t_data_unit tmp_data;
        copy_from_user((void*)&tmp_data, puser, len);
        if(tmp_data.opcode < eDU_MAX)
        {
            memcpy(&database[tmp_data.opcode], &tmp_data, sizeof(t_data_unit));
            return len;
        }
        else
        {
            return 0;
        }
    }
    else if(pfile->private_data == p_vol)
    {
        unsigned char tmp;
        copy_from_user((void*)&tmp, puser, len);
        //tipa_set_vol(tmp);
        return len;
    }
//    else if(pfile->private_data == p_8740)
//    {
//        unsigned char * pbuf;
//        pbuf = kmalloc(len, GFP_KERNEL);
//        copy_from_user((void*)pbuf, puser, len);
//        printk("8740 nwrite:%d\r\n", at_8740_write(pbuf, len));
//        kfree(pbuf);
//        return len;
//    }
//    else if(pfile->private_data == p_8740_wakeup)
//    {
//        printk("8740 wakeup\r\n");
//        at_8740_wakeup();
//        return len;
//    }
    else
    {
    }
    return 0;
}

static struct file_operations gp_fops =
{
    .owner = THIS_MODULE,
    .open = gp_open,
    .release = gp_close,
    .read = gp_read,
    .write = gp_write,
};

static int __init fpga_config_init(void)
{
#if 0
   D_IO_BASE = ioremap(D_IO_BASE_PHY, D_IO_LEN);
    if(!D_IO_BASE)
    {
        printk("[gen_prov] init gpio err\r\n");
        return ;
    }
#endif

    kset_vendor = kset_create_and_add("vendor", NULL, NULL);
    kobj_pa.kset = kset_vendor;
    kobject_init_and_add(&kobj_pa, &gen_ktype, NULL, "pa");

    major_num = register_chrdev(0, "fpga_chr", &gp_fops);
    if (major_num < 0)
    {
        printk("reg_chr_err\n");
        return 1;
    }

    p_class = class_create(THIS_MODULE, "fpga_class");
    if(p_class == NULL)
    {
        printk("class create err\n");
        return 1;
    }

    p_fpga = device_create(p_class, NULL, MKDEV(major_num, 0), NULL, "fpga_config");
    if(p_fpga == NULL)
    {
        printk("device_create err:fpga\n");
        return 1;
    }

    p_reg_debug = device_create(p_class, NULL, MKDEV(major_num, 1), NULL, "reg_debug");
    if(p_reg_debug == NULL)
    {
        printk("device_create err:reg_debug\n");
        return 1;
    }

    p_dsp_settings = device_create(p_class, NULL, MKDEV(major_num, 2), NULL, "dsp_settings");
    if(p_dsp_settings == NULL)
    {
        printk("device_create err:p_dsp_settings\n");
        return 1;
    }

    p_au = device_create(p_class, NULL, MKDEV(major_num, 3), NULL, "au_base");
    if(p_au == NULL)
    {
        printk("device_create err:au_base\n");
        return 1;
    }

    p_tk = device_create(p_class, NULL, MKDEV(major_num, 4), NULL, "touch_key");
    if(p_tk == NULL)
    {
        printk("device_create err:touch_key\n");
        return 1;
    }

    p_vol = device_create(p_class, NULL, MKDEV(major_num, 5), NULL, "vbox_vol");
    if(p_vol == NULL)
    {
        printk("device_create err:vbox_vol\n");
        return 1;
    }

//    p_8740 = device_create(p_class, NULL, MKDEV(major_num, 6), NULL, "atml_sec");
//    if(p_8740 == NULL)
//    {
//        printk("device_create err:p_8740\n");
//        return 1;
//    }

//    p_8740_wakeup = device_create(p_class, NULL, MKDEV(major_num, 7), NULL, "atml_sec_wake");
//    if(p_8740_wakeup == NULL)
//    {
//        printk("device_create err:p_8740\n");
//        return 1;
//    }

    return 0;
}
module_init(fpga_config_init);

static void __exit fpga_config_exit(void)
{
}
module_exit(fpga_config_exit);

/* Module information */
MODULE_AUTHOR("feitao");
MODULE_DESCRIPTION("fpga config");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:fpga");
