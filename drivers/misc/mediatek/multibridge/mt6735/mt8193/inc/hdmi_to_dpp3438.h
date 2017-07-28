#define MOTOR_MAGIC  'h'  
#define DPP3438_POWER_ON   _IOW(MOTOR_MAGIC,  1,int)
#define DPP3438_POWER_OFF   _IOW(MOTOR_MAGIC,  2,int)
#define DPP3438_CORRECT   _IOW(MOTOR_MAGIC,  3,int)
#define DPP3438_POWER_RATE   _IOW(MOTOR_MAGIC,  4,int)  //15W 8W
#define STM_POWER_CTL   _IOW(MOTOR_MAGIC,  5,int)
#define DPP3438_STATUS   _IOR(MOTOR_MAGIC,  6,int)
#define CHECK_CHARGER_TYPE   _IOR(MOTOR_MAGIC,  7,int)

//#define DPP3438_POWER   _IOW(MOTOR_MAGIC,  10,int)

struct ddp_config
{
	int l_throw;
	int m_throw;
	int l_DMD;
	int m_DMD;

	int l_PP;
	int m_PP;

	int power_rate;
	unsigned char buf[8];
};

struct stm_config
{
	int pin;
	int val;	
	unsigned char buf[8];
};


