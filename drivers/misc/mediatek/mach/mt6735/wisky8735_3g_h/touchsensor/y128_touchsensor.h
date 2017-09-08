#ifndef Y128_TOUCHSENSOR_H__
#define Y128_TOUCHSENSOR_H__

#define GPIO_TPPW_EN	( GPIO125 | 0x80000000 )
#define TS_I2C_NUMBER	2

#define TOUCHSENSOR_DEBUG
//#define TOUCHSENSOR_TEST
#ifdef TOUCHSENSOR_DEBUG

#define TS_TAG			"[touchsensor]"
#define TS_LOG(fmt, arg...)	printk(KERN_ERR TS_TAG fmt, ##arg)
#define TS_MSG(fmt, arg...)	printk(fmt, ##arg)
#define TS_ERR(fmt, arg...)	printk(KERN_ERR TS_TAG "ERROR,%s, %d: "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#define TS_FUNC(fmt, arg...) printk(TS_TAG "%s\n", __FUNCTION__)

#else

#define TS_TAG			"[touchsensor]"
#define TS_LOG(fmt, arg...)		//		
#define TS_MSG(fmt, arg...)		
#define TS_ERR(fmt, arg...)	
#define TS_FUNC(fmt, arg...)	
#endif

extern kal_bool backlight_status;

/****************

头部后脑勺：  yyd0
胸部：        yyd1

下巴:		  yyd2
后背:		  yyd3

右小臂:		  yyd4
右肩膀:		  yyd5

左小臂:		  yyd6
左肩膀:		  yyd7

右耳朵:		  yyd8
左耳朵:		  yyd9

左头部:		  yyd10
右头部:		  yyd11

*****************/

typedef enum {
   TOUCH_YYD0 = 0,	//头部左右耳朵		//con10  con11
   TOUCH_YYD1,		//头部后脑勺		//con2
   TOUCH_YYD2,		//下巴				//con4
   TOUCH_YYD3,		//左右肩膀			//con7	 con9
   TOUCH_YYD4,		//胸部				//con3
   TOUCH_YYD5,		//后背				//con5
   TOUCH_YYD6,		//左小臂			//con6	 con8
   TOUCH_YYD7,		//头部右耳朵
   TOUCH_YYD8,		//右肩膀
   TOUCH_YYD9,		//右小臂
   TOUCH_YYD10,		//头部左			//con12
   TOUCH_YYD11,		//头部右			//con13
   TOUCH_T_HEAD,	//					头部左右			//con12	 con13
   TOUCH_MAX,
} TouchCom;

typedef struct touchdata{
	kal_bool	down;
	kal_bool	valid;
}TOUCHDATA_T;

static char* TouchMap[TOUCH_MAX] = {"yyd0",  "yyd1",  "yyd2", 
									"yyd3",  "yyd4",  "yyd5", 
									"yyd6",  "yyd7",  "yyd8", 
									"yyd9",  "yyd10", "yyd11", "t_head"
								   };

#ifdef TOUCHSENSOR_TEST
static TouchCom touch_pos[TOUCH_MAX] = {TOUCH_T_HEAD, TOUCH_T_HEAD, 	TOUCH_T_HEAD, 
										  TOUCH_T_HEAD, TOUCH_T_HEAD,   TOUCH_T_HEAD, 
										  TOUCH_T_HEAD, TOUCH_T_HEAD,   TOUCH_T_HEAD,   
										  TOUCH_T_HEAD, TOUCH_T_HEAD,   TOUCH_T_HEAD,
										  TOUCH_T_HEAD};	
#else
static TouchCom touch_pos[TOUCH_MAX] = {  TOUCH_YYD0, 	TOUCH_YYD1, 	TOUCH_YYD2, 
										  TOUCH_YYD3,   TOUCH_YYD4,     TOUCH_YYD5, 
										  TOUCH_YYD6,   TOUCH_YYD7,     TOUCH_YYD8,   
										  TOUCH_YYD9,   TOUCH_YYD10,	TOUCH_YYD11,
										  TOUCH_T_HEAD
									   };
#endif
extern void commit_status(char *switch_name);
#endif /* Y128_TOUCHSENSOR_H__ */
