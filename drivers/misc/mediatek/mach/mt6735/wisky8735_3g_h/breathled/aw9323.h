#ifndef AW9323_H__
#define AW9323_H__

#define BLD_I2C_BUSNUM 3

#define BREATHLED_DEBUG

#ifdef BREATHLED_DEBUG

#define BLD_TAG			"[breathled]"
#define BLD_LOG(fmt, arg...)	printk(KERN_ERR BLD_TAG fmt, ##arg)
#define BLD_MSG(fmt, arg...)	printk(fmt, ##arg)
#define BLD_ERR(fmt, arg...)	printk(KERN_ERR BLD_TAG "ERROR,%s, %d: "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#define BLD_FUNC(fmt, arg...) printk(BLD_TAG "%s\n", __FUNCTION__)

#else

#define BLD_TAG			"[breathled]"
#define BLD_LOG(fmt, arg...)		//		
#define BLD_MSG(fmt, arg...)		
#define BLD_ERR(fmt, arg...)	
#define BLD_FUNC(fmt, arg...)	
#endif

#define BLD_I2C_SCL_L_PIN		(GPIO42 | 0x80000000)
#define BLD_I2C_SDA_L_PIN		(GPIO43 | 0x80000000)

#define BLD_I2C_SCL_R_PIN		(GPIO42 | 0x80000000)
#define BLD_I2C_SDA_R_PIN		(GPIO44 | 0x80000000)

#define BLD_I2C_ADDR_L		0xb6
#define BLD_I2C_ADDR_R		0xb6

#define BLD1_RSTN_PIN			(GPIO11 | 0x80000000)
#define BLD2_RSTN_PIN			(GPIO12 | 0x80000000)

#define BLD_VLED_3V3_EN			(GPIO82 | 0x80000000)

typedef enum{
	P00 = 0x24,
	P01 = 0x25,
	P02 = 0x26,
	P03 = 0x27,

	P04 = 0x28,
	P05 = 0x29,
	P06 = 0x2a,
	P07 = 0x2b,

	P10 = 0x20,
	P11 = 0x21,
	P12 = 0x22,
	P13 = 0x23,

	P14 = 0x2c,
	P15 = 0x2d,
	P16 = 0x2e,
	P17 = 0x2f,
	PMAX = 16,
}REG_PIO_E;

typedef enum{
	RED 	= 1 << 0,
	GREEN	= 1 << 1,
	BLUE	= 1 << 2,
	RG		= RED | GREEN,
	RB		= RED | BLUE,
	BG		= BLUE | GREEN,
	RGB		= RED | GREEN | BLUE,
	CLR_MAX	= 7,
}LED_CLR_E;

typedef enum{
	STP_LOW 	= 0,
	STP_MID,
	STP_HIGH,
	STP_MAX,
}LED_STEP_E;

typedef struct bld_clr_reg_map{
	kal_bool	led_sta;
	REG_PIO_E	reg;
	const LED_CLR_E color;
}BLD_CLR_REG_MAP_T;

typedef struct clr3_led{
	BLD_CLR_REG_MAP_T red;
	BLD_CLR_REG_MAP_T green;
	BLD_CLR_REG_MAP_T blue;
}CLR3_LED_T;

#define BLD_TASK_PERIOD_S                     0	
#define BLD_TASK_PERIOD_NS                    3*10*1000*1000

LED_STEP_E	rlf_step[3]	= {STP_LOW, STP_LOW, STP_LOW};

CLR3_LED_T	RBL[3] =			{
								[0] = 
								{	
									.red = {KAL_FALSE, P04, RED}, .green = {KAL_TRUE, P03, GREEN}, .blue = {KAL_FALSE, P02, BLUE} 
								},
								[1] = 
								{ 
									.red = {KAL_FALSE, P07, RED}, .green = {KAL_TRUE, P06, GREEN}, .blue = {KAL_FALSE, P05, BLUE} 
								},
								[2] = 
								{ 
									.red = {KAL_FALSE, P16, RED}, .green = {KAL_TRUE, P15, GREEN}, .blue = {KAL_FALSE, P14, BLUE} 
								},
								};

CLR3_LED_T	LBL[3] =			{
								[0] = 
								{ 
									.red = {KAL_FALSE, P04, RED}, .green = {KAL_TRUE, P03, GREEN}, .blue = {KAL_FALSE, P02, BLUE} 
								},
								[1] = 
								{ 
									.red = {KAL_FALSE, P07, RED}, .green = {KAL_TRUE, P06, GREEN}, .blue = {KAL_FALSE, P05, BLUE} 
								},
								[2] = 
								{ 
									.red = {KAL_FALSE, P16, RED}, .green = {KAL_TRUE, P15, GREEN}, .blue = {KAL_FALSE, P14, BLUE} 
								},
								};

CLR3_LED_T	FBL[5] =			{
								[0] = 
								{ 
									.red = {KAL_TRUE, P10, RED}, .green = {KAL_FALSE, P11, GREEN}, .blue = {KAL_FALSE, P12, BLUE} 
								},
								[1] = 
								{ 
									.red = {KAL_TRUE, P13, RED}, .green = {KAL_FALSE, P00, GREEN}, .blue = {KAL_FALSE, P01, BLUE} 
								},
								[2] = 
								{ 
									.red = {KAL_TRUE, P02, RED}, .green = {KAL_FALSE, P03, GREEN}, .blue = {KAL_FALSE, P04, BLUE} 
								},
								[3] = 
								{ 
									.red = {KAL_TRUE, P05, RED}, .green = {KAL_FALSE, P06, GREEN}, .blue = {KAL_FALSE, P07, BLUE} 
								},
								[4] = 
								{ 
									.red = {KAL_TRUE, P14, RED}, .green = {KAL_FALSE, P15, GREEN}, .blue = {KAL_FALSE, P16, BLUE} 
								},
								};

#define BREATH_MAX_STEP		66

static U8 breath_steps[STP_MAX][BREATH_MAX_STEP] = {
									{
									 0,   0,   3,   6,   9,   12,
									 15,  18,  21,  24,  27,  30,
									 34,  38,  42,  46,  50,  54,
									 60,  66,  72,  78,  84,  92,
									 102, 115, 125, 135, 145, 160,
									 180, 210, 255, 255, 210, 180,
									 160, 145, 135, 125, 115, 102, 
									 92,  84,  78,  72,  66,  60,  
									 54,  50,  46,  42,  38,  34,  
									 30,  27,  24,  21,  18,  15,
									 12,  9,   6,   3,   0,	  0,
									},

									{
									 0,   0,   3,	6,	 9,   12,
									 15,  18,  21,	24,  27,  30,
									 34,  38,  42,	46,  50,  54,
									 60,  66,  72,	78,  84,  92,
									 102, 115, 125, 135, 145, 160,
									 180, 210, 255, 255, 210, 180,
									 160, 145, 135, 125, 115, 102, 
									 92,  84,  78,	72,  66,  60,  
									 54,  50,  46,	42,  38,  34,  
									 30,  27,  24,	21,  18,  15,
									 12,  9,   6,	3,	 0,   0,
									},
								
									{
									 0,   0,	3,	 6,   9,   12,
									 15,  18,	21,  24,  27,  30,
									 34,  38,	42,  46,  50,  54,
									 60,  66,	72,  78,  84,  92,
									 102, 115, 125, 135, 145, 160,
									 180, 210, 255, 255, 210, 180,
									 160, 145, 135, 125, 115, 102, 
								 	 92,  84,	78,  72,  66,  60,	
									 54,  50,	46,  42,  38,  34,	
									 30,  27,	24,  21,  18,  15,
									 12,  9,	6,	 3,   0,   0,
									},
                                };


#endif /* AW9323_H__ */

