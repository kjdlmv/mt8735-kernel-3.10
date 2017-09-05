#ifdef BUILD_LK
#include <string.h>
#include <mt_gpio.h>
#else
#include <linux/string.h>
#if defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif
#endif
#include "lcm_drv.h"
#include <cust_gpio_usage.h>

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(800)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#define REGFLAG_ESCAPE_ID		(0x00)
#define REGFLAG_DELAY_MS_V3		(0xFF)
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size) 

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
}; 

static struct LCM_setting_table_V3{
    unsigned char id;    
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[128];
};

static void push_table(struct LCM_setting_table_V3 *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
			case REGFLAG_DELAY_MS_V3:
			    MDELAY(table[i].count);
				break;
            case REGFLAG_END_OF_TABLE :
                break;
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				//UDELAY(5);//soso add or it will fail to send register
       	}
    }
	
}
static LCM_setting_table_V3 lcm_initialization_setting[] = {

//JD9366+IVO8 initial code
	//Page0
	{0x15,0xE0,1,{0x00}},
	{0x15,0xE1,1,{0x93}},
	{0x15,0xE2,1,{0x65}},
	{0x15,0xE3,1,{0xF8}},
	
	//Page0
	{0x15,0xE0,1,{0x00}},
	{0x15,0x70,1,{0x02}},	
	{0x15,0x71,1,{0x23}},
	{0x15,0x72,1,{0x06}},	
	
	//--- Page1  ----//
	{0x15,0xE0,1,{0x01}},
	
	//Set VCOM
	{0x15,0x00,1,{0x00}},
	{0x15,0x01,1,{0x96}},
	//Set VCOM_Reverse
	{0x15,0x03,1,{0x00}},
	{0x15,0x04,1,{0x96}},	
	//Set Gamma Power, VGMP,VGMN,VGSP,VGSN  
	{0x15,0x17,1,{0x00}},
	{0x15,0x18,1,{0x75}},
	{0x15,0x19,1,{0x00}},
	{0x15,0x1A,1,{0x00}},
	{0x15,0x1B,1,{0x75}},  
	{0x15,0x1C,1,{0x00}},
	//Set Gate Power 
	{0x15,0x1F,1,{0x48}},					   
	{0x15,0x20,1,{0x23}},				   
	{0x15,0x21,1,{0x23}},				   
	{0x15,0x22,1,{0x0E}},
	
	{0x15,0x23,1,{0x00}},				   
	{0x15,0x24,1,{0x38}},				   
	{0x15,0x25,1,{0xD3}},
	//SETPANEL
	{0x15,0x37,1,{0x59}},	
	//SET RGBCYC
	{0x15,0x38,1,{0x05}},
	{0x15,0x39,1,{0x08}},
	{0x15,0x3A,1,{0x12}},	
	{0x15,0x3C,1,{0x78}},	
	{0x15,0x3E,1,{0x80}},
	{0x15,0x3F,1,{0x80}},
	//Set TCON
	{0x15,0x40,1,{0x06}},
	{0x15,0x41,1,{0xA0}},	

	//--- power voltage  ----//
	{0x15,0x55,1,{0x0F}},	
	{0x15,0x56,1,{0x01}},
	{0x15,0x57,1,{0x85}},
	{0x15,0x58,1,{0x0A}},
	{0x15,0x59,1,{0x0A}},
	{0x15,0x5A,1,{0x32}},	
	{0x15,0x5B,1,{0x0F}},
	
	//--- Gamma  ----//
	{0x15,0x5D,1,{0x70}},
	{0x15,0x5E,1,{0x6E}},
	{0x15,0x5F,1,{0x65}},
	{0x15,0x60,1,{0x5d}},
	{0x15,0x61,1,{0x5f}},
	
	{0x15,0x62,1,{0x51}},
	{0x15,0x63,1,{0x56}},
	{0x15,0x64,1,{0x3f}},
	{0x15,0x65,1,{0x55}},
	{0x15,0x66,1,{0x50}},
	
	{0x15,0x67,1,{0x4a}},
	{0x15,0x68,1,{0x68}},
	{0x15,0x69,1,{0x51}},
	{0x15,0x6A,1,{0x58}},
	{0x15,0x6B,1,{0x33}},
	
	{0x15,0x6C,1,{0x33}},
	{0x15,0x6D,1,{0x2a}},
	{0x15,0x6E,1,{0x1a}},
	{0x15,0x6F,1,{0x00}},
	{0x15,0x70,1,{0x7f}},
	
	{0x15,0x71,1,{0x76}},
	{0x15,0x72,1,{0x6d}},
	{0x15,0x73,1,{0x65}},
	{0x15,0x74,1,{0x66}},
	{0x15,0x75,1,{0x59}},
	
	{0x15,0x76,1,{0x5e}},
	{0x15,0x77,1,{0x47}},
	{0x15,0x78,1,{0x5d}},
	{0x15,0x79,1,{0x58}},
	{0x15,0x7A,1,{0x53}},
	
	{0x15,0x7B,1,{0x6f}},
	{0x15,0x7C,1,{0x59}},
	{0x15,0x7D,1,{0x5f}},
	{0x15,0x7E,1,{0x3b}},
	{0x15,0x7F,1,{0x3b}},
	
	{0x15,0x80,1,{0x31}},
	{0x15,0x81,1,{0x23}},
	{0x15,0x82,1,{0x00}},
		
	//Page2,1,{ for GIP
	{0x15,0xE0,1,{0x02}},
	//GIP_L Pin mapping  
	{0x15,0x00,1,{0x1F}},
	{0x15,0x01,1,{0x1F}},  
	{0x15,0x02,1,{0x13}},
	{0x15,0x03,1,{0x11}},
	{0x15,0x04,1,{0x0B}},
	
	{0x15,0x05,1,{0x0B}},
	{0x15,0x06,1,{0x09}},
	{0x15,0x07,1,{0x09}},
	{0x15,0x08,1,{0x07}},
	{0x15,0x09,1,{0x1F}},
	
	{0x15,0x0A,1,{0x1F}},
	{0x15,0x0B,1,{0x1F}},
	{0x15,0x0C,1,{0x1F}},
	{0x15,0x0D,1,{0x1F}},
	{0x15,0x0E,1,{0x1F}},
	
	{0x15,0x0F,1,{0x07}},
	{0x15,0x10,1,{0x05}},
	{0x15,0x11,1,{0x05}},
	{0x15,0x12,1,{0x01}},
	{0x15,0x13,1,{0x03}},
	
	{0x15,0x14,1,{0x1F}},
	{0x15,0x15,1,{0x1F}},
	
	//GIP_R Pin mapping
	{0x15,0x16,1,{0x1F}},
	{0x15,0x17,1,{0x1F}},
	{0x15,0x18,1,{0x12}},
	{0x15,0x19,1,{0x10}},
	{0x15,0x1A,1,{0x0A}},
	
	{0x15,0x1B,1,{0x0A}},
	{0x15,0x1C,1,{0x08}},
	{0x15,0x1D,1,{0x08}},
	{0x15,0x1E,1,{0x06}},
	{0x15,0x1F,1,{0x1F}},
	
	{0x15,0x20,1,{0x1F}},
	{0x15,0x21,1,{0x1F}},
	{0x15,0x22,1,{0x1F}},
	{0x15,0x23,1,{0x1F}},
	{0x15,0x24,1,{0x1F}},
	
	{0x15,0x25,1,{0x06}},
	{0x15,0x26,1,{0x04}},
	{0x15,0x27,1,{0x04}},
	{0x15,0x28,1,{0x00}},
	{0x15,0x29,1,{0x02}},
	
	{0x15,0x2A,1,{0x1F}},
	{0x15,0x2B,1,{0x1F}},
						  
	//GIP_L_GS Pin mapping
	{0x15,0x2C,1,{0x1F}},
	{0x15,0x2D,1,{0x1F}},	
	{0x15,0x2E,1,{0x00}}, 
	{0x15,0x2F,1,{0x02}}, 
	{0x15,0x30,1,{0x08}}, 
	
	{0x15,0x31,1,{0x08}}, 
	{0x15,0x32,1,{0x0A}}, 
	{0x15,0x33,1,{0x0A}}, 
	{0x15,0x34,1,{0x04}}, 
	{0x15,0x35,1,{0x1F}},
	
	{0x15,0x36,1,{0x1F}}, 
	{0x15,0x37,1,{0x1F}}, 
	{0x15,0x38,1,{0x1F}}, 
	{0x15,0x39,1,{0x1F}}, 
	{0x15,0x3A,1,{0x1F}},
	
	{0x15,0x3B,1,{0x04}}, 
	{0x15,0x3C,1,{0x06}}, 
	{0x15,0x3D,1,{0x06}}, 
	{0x15,0x3E,1,{0x12}}, 
	{0x15,0x3F,1,{0x10}}, 
	
	{0x15,0x40,1,{0x1F}}, 
	{0x15,0x41,1,{0x1F}},
	 
	//GIP_R_GS Pin mapping
	{0x15,0x42,1,{0x1F}},
	{0x15,0x43,1,{0x1F}},	
	{0x15,0x44,1,{0x01}}, 
	{0x15,0x45,1,{0x03}}, 
	{0x15,0x46,1,{0x09}}, 
	
	{0x15,0x47,1,{0x09}}, 
	{0x15,0x48,1,{0x0B}}, 
	{0x15,0x49,1,{0x0B}}, 
	{0x15,0x4A,1,{0x05}}, 
	{0x15,0x4B,1,{0x1F}}, 
	
	{0x15,0x4C,1,{0x1F}}, 
	{0x15,0x4D,1,{0x1F}}, 
	{0x15,0x4E,1,{0x1F}}, 
	{0x15,0x4F,1,{0x1F}}, 
	{0x15,0x50,1,{0x1F}},
	
	{0x15,0x51,1,{0x05}}, 
	{0x15,0x52,1,{0x07}}, 
	{0x15,0x53,1,{0x07}}, 
	{0x15,0x54,1,{0x13}}, 
	{0x15,0x55,1,{0x11}}, 
	
	{0x15,0x56,1,{0x1F}}, 
	{0x15,0x57,1,{0x1F}}, 
	
	//GIP Timing  
	{0x15,0x58,1,{0x40}}, 
	{0x15,0x59,1,{0x00}}, 
	{0x15,0x5A,1,{0x00}}, 
	{0x15,0x5B,1,{0x30}}, 
	{0x15,0x5C,1,{0x08}}, 
	
	{0x15,0x5D,1,{0x30}}, 
	{0x15,0x5E,1,{0x01}}, 
	{0x15,0x5F,1,{0x02}}, 
	{0x15,0x60,1,{0x30}}, 
	{0x15,0x61,1,{0x01}}, 
	
	{0x15,0x62,1,{0x02}}, 
	{0x15,0x63,1,{0x03}}, 
	{0x15,0x64,1,{0x6B}}, 
	{0x15,0x65,1,{0x75}}, 
	{0x15,0x66,1,{0x0D}},
	
	{0x15,0x67,1,{0x73}}, 
	{0x15,0x68,1,{0x0A}}, 
	{0x15,0x69,1,{0x03}}, 
	{0x15,0x6A,1,{0x6B}}, 
	{0x15,0x6B,1,{0x08}}, 
	
	{0x15,0x6C,1,{0x00}}, 
	{0x15,0x6D,1,{0x04}}, 
	{0x15,0x6E,1,{0x04}}, 
	{0x15,0x6F,1,{0x88}}, 
	{0x15,0x70,1,{0x00}},
	
	{0x15,0x71,1,{0x00}}, 
	{0x15,0x72,1,{0x06}}, 
	{0x15,0x73,1,{0x7B}}, 
	{0x15,0x74,1,{0x00}}, 
	{0x15,0x75,1,{0x3C}}, 
	
	{0x15,0x76,1,{0x00}}, 
	{0x15,0x77,1,{0x0D}}, 
	{0x15,0x78,1,{0x2C}}, 
	{0x15,0x79,1,{0x00}}, 
	{0x15,0x7A,1,{0x00}}, 
	
	{0x15,0x7B,1,{0x00}}, 
	{0x15,0x7C,1,{0x00}}, 
	{0x15,0x7D,1,{0x03}}, 
	{0x15,0x7E,1,{0x7B}}, 
	
	
	//Page1
	{0x15,0xE0,1,{0x01}},
	{0x15,0x0E,1,{0x01}},	//LEDON output VCSW2
	//{0x15,0x4A,1,{0x18}}, //test pattern
		
	//Page3
	{0x15,0xE0,1,{0x03}},
	{0x15,0x98,1,{0x2F}},	//From 2E to 2F,1,{ LED_VOL
	//Page4_ESD ADD 
	{0x15,0xE0,1,{0x04}}, 
	{0x15,0x2B,1,{0x2B}},
	{0x15,0x2E,1,{0x44}}, 
	
	{0x15,0xE0,1,{0x00}},
  {0x05, 0x11, 0, {0x00}},
  {REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 120, {}},
  {0x15, 0x35, 1, {0x00}},  //TE Output
  {REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 50, {}},       
  {0x05, 0x29, 0, 0x00},
  {REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 100, {}},
  {0x39, 0xBF, 3, {0x09, 0xB1, 0x7F}},    
};


static LCM_setting_table_V3 lcm_sleep_out_setting[] = {
		
	{0x05, 0x11, 0, {0x00}},
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 120, {}},	
  
	{0x05, 0x29,  0, {0x00}},	
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 20, {}},
	   
};
static LCM_setting_table_V3 lcm_sleep_in_setting[] = {
	                       
	// Display off sequence
	{0x05,0x28, 0, {0x00}},
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 20, {}},

        // Sleep Mode On
	{0x05,0x10, 0, {0x00}},
	{REGFLAG_ESCAPE_ID,REGFLAG_DELAY_MS_V3, 120, {}},
	
};


static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	// enable tearing-free
	//params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;
	//params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

	//params->dsi.mode	 = SYNC_PULSE_VDO_MODE;
	params->dsi.mode    = BURST_VDO_MODE;
	//params->dsi.mode	 = SYNC_EVENT_VDO_MODE; 

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	//params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	//params->dsi.data_format.trans_seq	 = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	//params->dsi.data_format.padding 	 = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	//params->dsi.packet_size=256;

	// Video mode setting		
	//params->dsi.intermediat_buffer_num = 2;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	//params->dsi.word_count=800*3;	


	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch =  11;
	params->dsi.vertical_frontporch = 8;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 8;
	params->dsi.horizontal_backporch =  8;
	params->dsi.horizontal_frontporch = 32;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK=225; 
	
}
static void lcm_init(void)
{
#if 0
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(6);//Must > 5ms
    SET_RESET_PIN(1);
    MDELAY(50);//Must > 50ms
#endif

#ifdef BUILD_UBOOT 
#elif defined(BUILD_LK) 
#else
		mt_set_gpio_mode(GPIO121 | 0x80000000, GPIO_MODE_00);
		mt_set_gpio_pull_enable(GPIO121 | 0x80000000, GPIO_PULL_ENABLE);
		mt_set_gpio_dir(GPIO121 | 0x80000000, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO121 | 0x80000000, GPIO_OUT_ONE);
		
		hwPowerOn(MT6328_POWER_LDO_VCAMD, VOL_1800, "1V8_LCD_VIO");
		hwPowerOn(MT6328_POWER_LDO_VCAMA, VOL_2800, "2V8_LCD_VCI");
		MDELAY(5);
		hwPowerDown(MT6328_POWER_LDO_VCAMA, "2V8_LCD_VCI");
		MDELAY(5);
		hwPowerOn(MT6328_POWER_LDO_VCAMA, VOL_2800, "2V8_LCD_VCI" );
#endif
	//hwPowerOn(MT6328_POWER_LDO_VUSB33, VOL_3300, "VUSB_LDO");
		mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
		mt_set_gpio_pull_enable(GPIO_LCM_RST, GPIO_PULL_ENABLE);
		mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
		
		mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
		MDELAY(10);
		mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
		MDELAY(10);
		mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
		MDELAY(150);
#if defined(BUILD_LK)
	  printf("jd9366_ivo8 build in LK %s\n", __func__);
#else
	  printk("jd9366_ivo8 build in Kernel %s\n", __func__);
#endif	
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table_V3), 1);
}


static void lcm_suspend(void)
{
#if 0
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(6);//Must > 5ms
    SET_RESET_PIN(1);
    MDELAY(50);//Must > 50ms
#endif
#if defined(BUILD_LK)
	  printf("jd9366_ivo8 build in LK %s\n", __func__);
#else
	  printk("jd9366_ivo8 build in Kernel %s\n", __func__);
#endif	
	mt_set_gpio_mode(GPIO121 | 0x80000000, GPIO_MODE_00);
	mt_set_gpio_pull_enable(GPIO121 | 0x80000000, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO121 | 0x80000000, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO121 | 0x80000000, GPIO_OUT_ZERO);
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_pull_enable(GPIO_LCM_RST, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(150);

	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table_V3), 1);
}


static void lcm_resume(void)
{
#if defined(BUILD_LK)
	  printf("jd9366_ivo8 build in LK %s\n", __func__);
#else
	  printk("jd9366_ivo8 build in Kernel %s\n", __func__);
#endif	
	mt_set_gpio_mode(GPIO121 | 0x80000000, GPIO_MODE_00);
	mt_set_gpio_pull_enable(GPIO121 | 0x80000000, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO121 | 0x80000000, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO121 | 0x80000000, GPIO_OUT_ONE);	
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_pull_enable(GPIO_LCM_RST, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(150);

	lcm_init();
	
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table_V3), 1);
}
static unsigned int lcm_compare_id(void)
{
	 int array[4];
	 char buffer[5];
	 char id_high=0;
	 char id_low=0;
	 int id1=0;
	 int id2=0;
	 return 0x9366;
	 SET_RESET_PIN(1);
	 MDELAY(10);
	 SET_RESET_PIN(0);
	 MDELAY(10);
	 SET_RESET_PIN(1);
	 MDELAY(120);
	 
	 array[0]=0x01FE1500;
	 dsi_set_cmdq(&array,1, 1);
	 
	 array[0] = 0x00013700;
	 dsi_set_cmdq(array, 1, 1);
	 read_reg_v2(0xde, buffer, 1);
	 
	 id_high = buffer[0];
	 read_reg_v2(0xdf, buffer, 1);
	 id_low = buffer[0];
	 id1 = (id_high<<8) | id_low;
	 
#if defined(BUILD_LK)
	  printf("jd9366_ivo8 %s id1 = 0x%04x, id2 = 0x%04x\n", __func__, id1,id2);
#else
	  printk("jd9366_ivo8 %s id1 = 0x%04x, id2 = 0x%04x\n", __func__, id1,id2);
#endif
	// return (0x6820 == id1)?1:0;
	return id1;
}

LCM_DRIVER jd9366_ivo8_lcm_drv = 
{
    .name			= "jd9366_ivo8",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

