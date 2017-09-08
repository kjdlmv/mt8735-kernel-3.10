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

ͷ�������ף�  yyd0
�ز���        yyd1

�°�:		  yyd2
��:		  yyd3

��С��:		  yyd4
�Ҽ��:		  yyd5

��С��:		  yyd6
����:		  yyd7

�Ҷ���:		  yyd8
�����:		  yyd9

��ͷ��:		  yyd10
��ͷ��:		  yyd11

*****************/

typedef enum {
   TOUCH_YYD0 = 0,	//ͷ�����Ҷ���		//con10  con11
   TOUCH_YYD1,		//ͷ��������		//con2
   TOUCH_YYD2,		//�°�				//con4
   TOUCH_YYD3,		//���Ҽ��			//con7	 con9
   TOUCH_YYD4,		//�ز�				//con3
   TOUCH_YYD5,		//��				//con5
   TOUCH_YYD6,		//��С��			//con6	 con8
   TOUCH_YYD7,		//ͷ���Ҷ���
   TOUCH_YYD8,		//�Ҽ��
   TOUCH_YYD9,		//��С��
   TOUCH_YYD10,		//ͷ����			//con12
   TOUCH_YYD11,		//ͷ����			//con13
   TOUCH_T_HEAD,	//					ͷ������			//con12	 con13
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
