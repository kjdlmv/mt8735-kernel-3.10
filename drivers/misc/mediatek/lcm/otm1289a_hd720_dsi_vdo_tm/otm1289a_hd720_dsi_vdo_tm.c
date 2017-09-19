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

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_OTM1289 (0x1289)


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x100   // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   

#define   LCM_DSI_CMD_MODE							0

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {

	   
	{0x00,1,{0x00}},		//EXTC=1
        {0xff,3,{0x12,0x89,0x01}},

        {0x00,1,{0x80}},		//CMD2 enable
        {0xff,2,{0x12,0x89}},

        {0x00,1,{0x90}},         	//MIPI 4:0xb0, 3:0xa0, 2:0x90
        {0xff,1,{0x90}},

//-------------------- panel setting --------------------------------//
        {0x00,1,{0x80}},             //TCON Setting
        {0xc0,8,{0x4a,0x00,0x10,0x10,0x96,0x01,0x68,0x40}},

        {0x00,1,{0x90}},             //Panel Timing Setting
        {0xc0,3,{0x3b,0x01,0x09}},

        {0x00,1,{0x8c}},             //column inversion
        {0xc0,1,{0x00}},

        {0x00,1,{0x80}},             //frame rate:60Hz
        {0xc1,1,{0x33}},




//-------------------- power setting --------------------------------//
        {0x00,1,{0x85}},             //VGH=6x, VGL=-5x, VGH=12V, VGL=-12V
        {0xc5,3,{0x09,0x0a,0x19}},

	{0x00,1,{0x00}},             //GVDD=4.85V, NGVDD=-4.85V
	{0xd8,2,{0x28,0x28}},    
	{0x00,1,{0x00}},             //GVDD=4.85V, NGVDD=-4.85V
	{0xd9,2,{0x80,0x39}},     
 
        {0x00,1,{0x84}},             //chopper
        {0xC4,1,{0x02}},

        {0x00,1,{0x93}},             //pump option
        {0xC4,1,{0x04}},
        
        {0x00,1,{0x96}},  		//VCL regulator
	{0xF5,1,{0xE7}},				

	{0x00,1,{0xA0}},    		//pump3 off
	{0xF5,1,{0x4A}},
	
        {0x00,1,{0x8a}},             //blank frame
	{0xc0,1,{0x11}},
  
  	{0x00,1,{0x83}},             //vcom active
  	{0xF5,1,{0x81}},

//-------------------- panel enmode control --------------------//
        {0x00,1,{0x80}},		//panel timing state control
	{0xCB,15,{0x14,0x14,0x14,0x14,0x14,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

        {0x00,1,{0x90}},		//panel timing state control
        {0xCB,7,{0x00,0x00,0x00,0x14,0x14,0x00,0x00}},
       
//-------------------- panel u2d/d2u mapping control --------------------//
        {0x00,1,{0x80}},		//panel timing state control
        {0xCC,14,{0x07,0x05,0x0F,0x0D,0x0B,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

        {0x00,1,{0x90}},		//panel timing state control
	{0xCC,15,{0x00,0x00,0x00,0x01,0x03,0x00,0x00,0x08,0x06,0x10,0x0E,0x0C,0x0A,0x00,0x00}},

        {0x00,1,{0xA0}},		//panel timing state control
        {0xCC,13,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x04,0x00,0x00}},

        {0x00,1,{0xB0}},		//panel timing state control
        {0xCC,14,{0x02,0x04,0x0E,0x10,0x0A,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

        {0x00,1,{0xC0}},		//panel timing state control
	{0xCC,15,{0x00,0x00,0x00,0x08,0x06,0x00,0x00,0x01,0x03,0x0D,0x0F,0x09,0x0B,0x00,0x00}},

        {0x00,1,{0xD0}},		//panel timing state control
        {0xCC,13,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x05,0x00,0x00}},

//-------------------- goa timing setting --------------------//


{0x00,1,{0x80}},//panel VST setting
{0xCE,6,{0x87,0x03,0x10,0x86,0x85,0x84}},

{0x00,1,{0x90}},//panel VEND setting
{0xCE,9,{0x34,0xFB,0x10,0x04,0xFC,0x04,0xFD,0x04,0xFE}},

{0x00,1,{0xA0}},//panel CLKA setting
{0xCE,15,{0x30,0x85,0x82,0x00,0x10,0x00,0x84,0x81,0x00,0x83, 0x80,0x00,0x82,0x00,0x00}},


{0x00,1,{0xB0}},//panel CLKB setting
{0xCE,15,{0x30,0x81,0x01,0x00,0x10,0x00,0x80,0x02,0x00,0x00,0x03,0x00,0x01,0x04,0x00}},

{0x00,1,{0xC0}},//panel CLKB setting
{0xCE,15,{0x30,0x02,0x05,0x00,0x10,0x00,0x03,0x06,0x00,0x04, 0x07,0x00,0x05,0x08,0x00}},

{0x00,1,{0xE0}},
{0xCE,8,{0x0a,0x04,0xfc,0x00,0x00,0x0a,0x04,0xfc}},
  
{0x00,1,{0xF0}},//panel ECLK setting
{0xCE,6,{0x3d,0x20,0x01,0x00,0x00,0x00}},


////////................................................gama...................////////////////////////////////////////
	//{0x00,1,{0x00}}, //panel CLKB setting////////////////////////
	//{0xE1,16,{0x32,0x45,0x4f,0x62,0x6e,0x86,0x7d,0x8d,0x6e,0x5f,0x6a,0x56,0x41,0x31,0x22,0x12}},
	//{0x00,1,{0x00}}, //panel CLKB setting
	//{0xE2,16,{0x32,0x45,0x4f,0x62,0x6e,0x86,0x7d,0x8d,0x6e,0x5f,0x6a,0x56,0x41,0x31,0x22,0x12}},
    {0x00,1,{0x00}}, //panel CLKB setting////////////////////////
	{0xE1,16,{0x32,0x46,0x53,0x64,0x6e,0x87,0x7f,0x8e, 0x6d,0x5e,0x6a,0x52,0x3f,0x2b,0x1d,0x12}},
	{0x00,1,{0x00}}, //panel CLKB setting
	{0xE2,16,{0x32,0x46,0x53,0x65,0x6e,0x86,0x7e,0x8f, 0x6e,0x5e,0x6a,0x53,0x3f,0x2b,0x1d,0x12}},


        {0x00,1,{0x00}},             //CMD2 disable
        {0xff,3,{0xff,0xff,0xff}},

	//{0x35, 1, {0x00}},

	{0x11,1,{0x00}},  
	{REGFLAG_DELAY,120,{}},

	{0x29,1,{0x00}},//Display ON 
	{REGFLAG_DELAY,20,{}},	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 100, {}},

    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				MDELAY(2);
       	}
    }
	
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

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
	params->dbi.te_mode                             = LCM_DBI_TE_MODE_DISABLED;//LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity                    = LCM_POLARITY_RISING;

	#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
	#else
	params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
	#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;//LCM_FOUR_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;//0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=720*3;	

		
    params->dsi.vertical_sync_active                = 8;
    params->dsi.vertical_backporch                  = 20;
    params->dsi.vertical_frontporch                 = 20;
    params->dsi.vertical_active_line                = FRAME_HEIGHT; 
    params->dsi.horizontal_sync_active              = 10;
    params->dsi.horizontal_backporch                = 45;
    params->dsi.horizontal_frontporch               = 45;
    params->dsi.horizontal_active_pixel            = FRAME_WIDTH;
    // Bit rate calculation
#if 0
    params->dsi.pll_div1=1;     // div1=0,1,2,3;div1_real=1,2,4,4
    params->dsi.pll_div2=1;     // div2=0,1,2,3;div2_real=1,2,4,4
    params->dsi.fbk_sel=1;       // fbk_sel=0,1,2,3;fbk_sel_real=1,2,4,4
    params->dsi.fbk_div =30;        // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)    
#else
    params->dsi.PLL_CLOCK=400;//227;//254(124 0k)//247  //220
#endif

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

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
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
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_pull_enable(GPIO_LCM_RST, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(150);

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

}

static void lcm_resume(void)
{
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	mt_set_gpio_pull_enable(GPIO_LCM_RST, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(150);
	//lcm_initialization_setting[1].para_list[0] += 2;
	lcm_init();
	
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
	unsigned int id0,id1,id=0;
	unsigned char buffer[5];
	unsigned int array[16];  


	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20); 

	array[0] = 0x00053700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xA1, buffer, 5);	//018B1283ff
	id0 = buffer[2];
	id1 = buffer[3];
	id=(id0<<8)|id1;
	
    #ifdef BUILD_LK
		printf("%s, LK otm1289a debug: otm1289a id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel otm1289a horse debug: otm1289a id = 0x%08x\n", __func__, id);
    #endif
	return 1;
	return id0;
	
    if(id == LCM_ID_OTM1289)
    	return 1;
    else
        return 0;
}


LCM_DRIVER otm1289a_dsi_hd_drv = 
{
    .name			= "otm1289a_dsi_hd_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
//     .init_power		= lcm_init_power,
 //    .resume_power = lcm_resume_power,
 //    .suspend_power = lcm_suspend_power,
	//.esd_check 	= lcm_esd_check,
	//.esd_recover	= lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};
