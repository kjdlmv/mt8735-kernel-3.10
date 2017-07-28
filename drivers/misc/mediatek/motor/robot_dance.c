#include<linux/time.h>
#include<uapi/linux/time.h>
#include<linux/unistd.h>
#include<motor_drv.h>
#include<robot_dance.h>
#include<linux/string.h>
#include<linux/signal.h>

#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include<linux/kthread.h>

#include <asm/uaccess.h>
#include <linux/hwmsen_dev.h>
#include <asm/uaccess.h>	/* copy_*_user */
#include <linux/semaphore.h>  
#include <linux/device.h>   /*class_create*/ 
#include <linux/slab.h>

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>
#include<linux/delay.h>
//#include<asm-generic/delay.h>
#include<linux/timer.h>
#include<linux/module.h>
#include<linux/jiffies.h>

MODULE_LICENSE("GPL");
#include "motor_drv.h"

static unsigned int dance_preaction_num=0;

extern unsigned int stopcmd;
extern unsigned int stop_flag;
extern unsigned int h_stop_flag;
/*************************************************************/
/*************************************************************/
/*************************************************************/

int MOTOR_FORWARD_P(unsigned int fast,unsigned int slow) ;
int MOTOR_BACK_P(unsigned int fast,unsigned int slow);		   
int MOTOR_LEFT_P(unsigned int fast,unsigned int slow);
int MOTOR_RIGHT_P(unsigned int fast,unsigned int slow);
int MOTOR_HEAD_BACK_P(unsigned int fast,unsigned int slow);
int MOTOR_HEAD_FORWARD_P(unsigned int fast,unsigned int slow);   
int MOTOR_HEAD_LEFT_P(unsigned int fast,unsigned int slow);   			
int MOTOR_HEAD_RIGHT_P(unsigned int fast,unsigned int slow);
int MOTOR_HEAD_RIGHT_PP(unsigned int fast,unsigned int slow);
int MOTOR_HEAD_LEFT_PP(unsigned int fast,unsigned int slow);


void MOTOR_STOP_P(void);
void MOTOR_V_STOP_P(void); 		   
void MOTOR_H_STOP_P(void); 

void POWER_EN_P(void);
void POWER_CLOSE_P(void);

int MOTOR_LEFT_CLOCKDIR(void);
int MOTOR_LEFT_ANTICLOCK(void);
int MOTOR_RIGHT_ANTICLOCK(void);
int MOTOR_RIGHT_CLOCKDIR(void);

void POWER_EN_P(void) 			
{		
	mt_set_gpio_out(GPIO101_PIN, 1);  
	mt_set_gpio_out(GPIO102_PIN, 1);
	
}

void POWER_CLOSE_P(void)   		
{
	mt_set_gpio_out(GPIO101_PIN, 0);  
	mt_set_gpio_out(GPIO102_PIN, 0);
}

int MOTOR_BACK_P(unsigned int fast,unsigned int slow) 
{    
 if((1==fast)&&(0==slow))
{
	mt_set_gpio_out(GPIO44_PIN, 1);
	mt_set_gpio_out(GPIO21_PIN, 1);
	mt_set_gpio_out(GPIO10_PIN, 0); 
	mt_set_gpio_out(GPIO60_PIN, 0); 	
}
else if ((0==fast)&&(1==slow))
{
    stop_flag=0;
while(!stop_flag)
{
   
      mt_set_gpio_out(GPIO44_PIN, 1);
      mt_set_gpio_out(GPIO21_PIN, 1);
      mt_set_gpio_out(GPIO10_PIN, 0); 
      mt_set_gpio_out(GPIO60_PIN, 0); 
      msleep(1);
      mt_set_gpio_out(GPIO44_PIN, 0);
      mt_set_gpio_out(GPIO21_PIN, 0);
      mt_set_gpio_out(GPIO10_PIN, 0); 
      mt_set_gpio_out(GPIO60_PIN, 0); 
      msleep(1);   
}
return 0;
}
}

 void MOTOR_BACK_PP(void)
 {
 unsigned int i_idex;
 for(i_idex=0;i_idex<=60;i_idex++)
 {
      mt_set_gpio_out(GPIO44_PIN, 1);
      mt_set_gpio_out(GPIO21_PIN, 1);
      mt_set_gpio_out(GPIO10_PIN, 0); 
      mt_set_gpio_out(GPIO60_PIN, 0); 
      msleep(1);
      mt_set_gpio_out(GPIO44_PIN, 0);
      mt_set_gpio_out(GPIO21_PIN, 0);
      mt_set_gpio_out(GPIO10_PIN, 0); 
      mt_set_gpio_out(GPIO60_PIN, 0); 
      msleep(2);   
 }
 }

 int MOTOR_FORWARD_P(unsigned int fast,unsigned int slow)   		
{
if ((1==fast)&&(0==slow))
{

    	mt_set_gpio_out(GPIO10_PIN, 1); 
	mt_set_gpio_out(GPIO60_PIN, 1); 
	mt_set_gpio_out(GPIO44_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 0);
}
 else if((0==fast)&&(1==slow))
{
	 stop_flag=0;

while(!stop_flag)
{
    	mt_set_gpio_out(GPIO10_PIN, 1); 
	mt_set_gpio_out(GPIO60_PIN, 1); 
	mt_set_gpio_out(GPIO44_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 0);
     	msleep(1);
	mt_set_gpio_out(GPIO10_PIN, 0); 
	mt_set_gpio_out(GPIO60_PIN, 0); 
	mt_set_gpio_out(GPIO44_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 0);
	msleep(1);
}
return 0;

}
}

 int MOTOR_RIGHT_P(unsigned int fast,unsigned int slow)   			
{

if ((1==fast)&&(0==slow))
{
	mt_set_gpio_out(GPIO44_PIN, 0); 
	mt_set_gpio_out(GPIO10_PIN, 1); 
	mt_set_gpio_out(GPIO60_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 1);
}
else if ((0==fast)&&(1==slow))
{
	stop_flag=0;

while(!stop_flag)
{
	mt_set_gpio_out(GPIO44_PIN, 0); 
	mt_set_gpio_out(GPIO10_PIN, 1); 
	mt_set_gpio_out(GPIO60_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 1);
	msleep(1);
	mt_set_gpio_out(GPIO44_PIN, 0); 
	mt_set_gpio_out(GPIO10_PIN, 0); 
	mt_set_gpio_out(GPIO60_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 0);
	msleep(1);
}
return 0;
}
}

 int MOTOR_RIGHT_CLOCKDIR(void)
 {
	 mt_set_gpio_out(GPIO44_PIN, 1); 
	 mt_set_gpio_out(GPIO10_PIN, 0); 
	 mt_set_gpio_out(GPIO60_PIN, 0); 
	 mt_set_gpio_out(GPIO21_PIN, 0);
 }

 int MOTOR_RIGHT_ANTICLOCK(void)
 {
     	 mt_set_gpio_out(GPIO44_PIN, 0); 
	 mt_set_gpio_out(GPIO10_PIN, 1); 
	 mt_set_gpio_out(GPIO60_PIN, 0); 
	 mt_set_gpio_out(GPIO21_PIN, 0);
 }

 int  MOTOR_LEFT_P(unsigned int fast,unsigned int slow)   			
{

 if ((1==fast)&&(0==slow))
{
	mt_set_gpio_out(GPIO60_PIN, 1); 
	mt_set_gpio_out(GPIO44_PIN, 1); 
	mt_set_gpio_out(GPIO10_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 0);
}
else if ((0==fast)&&(1==slow))
{
	stop_flag=0;

 while(!stop_flag)
{
	mt_set_gpio_out(GPIO60_PIN, 1); 
	mt_set_gpio_out(GPIO44_PIN, 1); 
	mt_set_gpio_out(GPIO10_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 0);
     	msleep(1);
	mt_set_gpio_out(GPIO60_PIN, 0); 
	mt_set_gpio_out(GPIO44_PIN, 0); 
	mt_set_gpio_out(GPIO10_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 0);
	msleep(1);

}
return 0;
}
}

  int MOTOR_LEFT_CLOCKDIR(void)      //clock  direction
 {
	 mt_set_gpio_out(GPIO60_PIN, 1); 
	 mt_set_gpio_out(GPIO21_PIN, 0);
	 mt_set_gpio_out(GPIO44_PIN, 0); 
	 mt_set_gpio_out(GPIO10_PIN, 0);
 }
  
  int MOTOR_LEFT_ANTICLOCK(void)    //anticlock direction
  {    
  	 mt_set_gpio_out(GPIO60_PIN, 0); 
	 mt_set_gpio_out(GPIO21_PIN, 1);
	 mt_set_gpio_out(GPIO44_PIN, 0); 
	 mt_set_gpio_out(GPIO10_PIN, 0);
  }

 void MOTOR_STOP_P()   			
{
	stop_flag=1;
	mt_set_gpio_out(GPIO44_PIN, 0); 
	mt_set_gpio_out(GPIO10_PIN, 0); 
	mt_set_gpio_out(GPIO60_PIN, 0); 
	mt_set_gpio_out(GPIO21_PIN, 0);
}

 int MOTOR_HEAD_BACK_P(unsigned int fast,unsigned int slow)   			
{
  if ((1==fast)&&(0==slow))
  {
       	mt_set_gpio_out(GPIO95_PIN, 1); 
	mt_set_gpio_out(GPIO96_PIN, 0);
  }
else if ((0==fast)&&(1==slow))
  {
      h_stop_flag=0;
  while(!h_stop_flag)
  {
       	mt_set_gpio_out(GPIO95_PIN, 1); 
	mt_set_gpio_out(GPIO96_PIN, 0);
     	msleep(1);
	mt_set_gpio_out(GPIO95_PIN, 0); 
	mt_set_gpio_out(GPIO96_PIN, 0);
	msleep(1);
  }
  return 0;
  }
}

 int MOTOR_HEAD_RIGHT_P(unsigned int fast,unsigned int slow)   			
{


 if ((1==fast)&&(0==slow))
 {
      	mt_set_gpio_out(GPIO42_PIN, 1); 
	mt_set_gpio_out(GPIO44_PIN, 0); 
 }
 else if ((0==fast)&&(1==slow))
 {
      h_stop_flag=0;
 while(!h_stop_flag)
 {
      	mt_set_gpio_out(GPIO42_PIN, 1); 
	mt_set_gpio_out(GPIO44_PIN, 0); 
     	msleep(1);
	mt_set_gpio_out(GPIO42_PIN,0); 
	mt_set_gpio_out(GPIO44_PIN, 0); 
	msleep(1);
 }
 return 0;
}
}


 int MOTOR_HEAD_RIGHT_PP(unsigned int fast,unsigned int slow)			 
 {
   int i;
   for (i=0;i<=100;i++)
   {
        mt_set_gpio_out(GPIO42_PIN, 1); 
	 mt_set_gpio_out(GPIO44_PIN, 0); 
	 msleep(1);
	 mt_set_gpio_out(GPIO42_PIN,0); 
	 mt_set_gpio_out(GPIO44_PIN, 0); 
	 msleep(1);
   }
  return 0;
 }


 int MOTOR_HEAD_LEFT_P(unsigned int fast,unsigned int slow)   			
{
 if ((1==fast)&&(0==slow))
{
	 mt_set_gpio_out(GPIO42_PIN, 0); 
        mt_set_gpio_out(GPIO44_PIN, 1); 

}
else if ((0==fast)&&(1==slow))
{
	h_stop_flag=0;

while(!h_stop_flag)
{
	mt_set_gpio_out(GPIO42_PIN, 0); 
	mt_set_gpio_out(GPIO44_PIN, 1); 
	msleep(1);
	mt_set_gpio_out(GPIO42_PIN, 0); 
	mt_set_gpio_out(GPIO44_PIN, 0); 
	msleep(1);
}
return 0;
}
}

 int MOTOR_HEAD_LEFT_PP(unsigned int fast,unsigned int slow)			 
 {
   int i;
   for (i=1;i<=100;i++)
   {
        mt_set_gpio_out(GPIO42_PIN, 1); 
	 mt_set_gpio_out(GPIO44_PIN, 0); 
	 msleep(1);
	 mt_set_gpio_out(GPIO42_PIN,0); 
	 mt_set_gpio_out(GPIO44_PIN, 0); 
	 msleep(1);
   }
  return 0;
 }

 int MOTOR_HEAD_FORWARD_P(unsigned int fast,unsigned int slow)   			
{	 

 if ((1==fast)&&(0==slow))
 {
	mt_set_gpio_out(GPIO95_PIN, 0); 
	mt_set_gpio_out(GPIO96_PIN, 1);
 }
else if ((0==fast)&&(1==slow))
 {
      h_stop_flag=0;
 while(!h_stop_flag)
 {
	mt_set_gpio_out(GPIO95_PIN, 0); 
	mt_set_gpio_out(GPIO96_PIN, 1);
     	msleep(1);
	mt_set_gpio_out(GPIO95_PIN, 0); 
	mt_set_gpio_out(GPIO96_PIN, 0);
	msleep(1);
}
 return 0;
}
}

 void MOTOR_V_STOP_P() 			
{	
       h_stop_flag=1;
	mt_set_gpio_out(GPIO95_PIN, 0); 
	mt_set_gpio_out(GPIO96_PIN, 0);
}

void  MOTOR_H_STOP_P(void)   			
 {    
        h_stop_flag=1;
	 mt_set_gpio_out(GPIO42_PIN, 0); 
	 mt_set_gpio_out(GPIO44_PIN, 0);
 }

void Motor_Test(void)
{

while(1)
{
    mt_set_gpio_out(GPIO44_PIN, 1);
    mt_set_gpio_out(GPIO10_PIN, 0); 
    mt_set_gpio_out(GPIO60_PIN, 1); 
    mt_set_gpio_out(GPIO21_PIN, 0);
    mt_set_gpio_out(GPIO95_PIN, 1); 
    mt_set_gpio_out(GPIO96_PIN, 0);
    mt_set_gpio_out(GPIO42_PIN, 1); 
    mt_set_gpio_out(GPIO44_PIN, 0);

    mt_set_gpio_out(GPIO44_PIN, 0);
    mt_set_gpio_out(GPIO10_PIN, 1); 
    mt_set_gpio_out(GPIO60_PIN, 0); 
    mt_set_gpio_out(GPIO21_PIN, 1);
    mt_set_gpio_out(GPIO95_PIN, 0); 
    mt_set_gpio_out(GPIO96_PIN, 1);
    mt_set_gpio_out(GPIO42_PIN, 0); 
    mt_set_gpio_out(GPIO44_PIN, 1);

}

}

void POWER_EN(void)   			
{		
	mt_set_gpio_out(GPIO101_PIN, 1);
	mt_set_gpio_out(GPIO102_PIN, 1); 
}

void POWER_CLOSE(void)			
{
	mt_set_gpio_out(GPIO101_PIN, 0); 
	mt_set_gpio_out(GPIO102_PIN, 0); 
}

void POWER_H_CLOSE(void)
{
	mt_set_gpio_out(GPIO99_PIN, 0);
}

void POWER_H_OPEN(void)
{
	mt_set_gpio_out(GPIO99_PIN, 1);
}

/************************************************************/
/************************************************************/
/************************************************************/
  //Time period:2.4s
 void RobotPreAction_P(unsigned int fast,unsigned int slow)
 {
unsigned int factor;
unsigned fast_temp=0;
unsigned slow_temp=0;

if ((1==fast)&&(0==slow))
{
factor=0;
}

if ((1==slow)&&(0==fast))
{
factor=1;
}

fast_temp=fast;
slow_temp=slow;

       #define delaytime400ms   200*(2-factor)

	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
	msleep(100);
	MOTOR_V_STOP_P();
	msleep(200);
	MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
	msleep(100);
	MOTOR_V_STOP_P();
	msleep(200);
 
 }
 
 /*****************************************************/
 //第一套机器人舞蹈动作
 //fast=1,slow=0,    机器人处于快速工作状态
 //fast=0,slow=1，机器人处于慢速工作状态
 //Time period:18.7s
void RobotDanceAction_11_P(unsigned int fast,unsigned int slow)
{	
unsigned int i_idex;
unsigned int factor;//速度因子

unsigned fast_temp=0;
unsigned slow_temp=0;
fast_temp=fast;
slow_temp=slow;


if ((1==fast_temp)&&(0==slow_temp))
{
factor=0;
}

if ((1==slow_temp)&&(0==fast_temp))
{
factor=1;
}

#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)

if (!stopcmd)
{

       MOTOR_FORWARD_P(fast_temp,slow_temp);
       msleep(600);
	MOTOR_STOP_P();
	msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_BACK_P(fast_temp,slow_temp);
	msleep(600);
	MOTOR_STOP_P();
	msleep(500);
     if (stopcmd)
     	{
     	return;
     	}

	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
     	{
     	return;
     	}

	MOTOR_LEFT_P(fast_temp,slow_temp);
	msleep(1500);
	MOTOR_STOP_P();	
	msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_LEFT_P(fast_temp,slow_temp);
	msleep(1500);
	MOTOR_STOP_P();
	msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_RIGHT_P(fast_temp,slow_temp);
	msleep(1500);
	MOTOR_STOP_P();
	msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_RIGHT_P(fast_temp,slow_temp);
	msleep(1500);
	MOTOR_STOP_P();
	msleep(500);
	 if (stopcmd)
     	{
     	return;
     	}
	MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
	msleep(100);
       MOTOR_V_STOP_P();	
	msleep(200);
	MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
	msleep(100);
	MOTOR_V_STOP_P();
	msleep(200);
	 if (stopcmd)
     	{
     	return;
   	}
       for(i_idex=0;i_idex<=1;i_idex++)
    	 {
    	      if (stopcmd)
    	      	{
    	      	break;
    	      	}

		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);		
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);		
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
    	 }
	if (stopcmd)
     	{
     	return;
     	}	
	MOTOR_RIGHT_P(fast_temp,slow_temp);
	msleep(3000);
	MOTOR_STOP_P();
	msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_RIGHT_P(fast_temp,slow_temp);
	msleep(3000);
	MOTOR_STOP_P();
	msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_LEFT_P(fast_temp,slow_temp);
	msleep(3000);
	MOTOR_STOP_P();
	msleep(500);
	 if (stopcmd)
     	{
     	return;
     	}
	MOTOR_LEFT_P(fast_temp,slow_temp);
	msleep(3000);
	MOTOR_STOP_P();
	msleep(500);
	 if (stopcmd)
     	{
     	return;
     	}
	
	MOTOR_FORWARD_P(fast_temp,slow_temp);
	msleep(600);
	MOTOR_STOP_P();
	msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_BACK_P(fast_temp,slow_temp);	
	msleep(600);
	MOTOR_STOP_P();
	msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
       for (i_idex=0;i_idex<=5;i_idex++)
       	{
       	if (stopcmd)
     	{
     	break;
     	}
       MOTOR_RIGHT_P(fast_temp,slow_temp);
	msleep(500);
	MOTOR_STOP_P();
	msleep(500);
       	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_RIGHT_P(fast_temp,slow_temp);
	msleep(500);
	MOTOR_STOP_P();
	msleep(500);
       	}
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_H_STOP_P();
	MOTOR_V_STOP_P();
	MOTOR_STOP_P();
	}
}
 
 /******************************************************/
 //第二套机器人舞蹈动作
 //fast=1,slow=0,    机器人处于快速工作状态
 //fast=0,slow=1，机器人处于慢速工作状态
//Time period:14.1s
 void RobotDanceAction_21_P(unsigned int fast,unsigned int slow)
 {	
unsigned int i_idex;
unsigned int factor;//速度因子
unsigned fast_temp=0;
unsigned slow_temp=0;
fast_temp=fast;
slow_temp=slow;

if ((1==fast_temp)&&(0==slow_temp))
{
factor=0;
}

if ((1==slow_temp)&&(0==fast_temp))
{
factor=1;
}

unsigned int result;
#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)
 
 if (!stopcmd)
 {
	 MOTOR_LEFT_P(fast_temp,slow_temp);
	 msleep(3000);
	 MOTOR_STOP_P();
	 msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	 MOTOR_LEFT_P(fast_temp,slow_temp);
	 msleep(3000);
	 MOTOR_STOP_P();
	 msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
     	{
     	return;
     	}
	 MOTOR_RIGHT_P(fast_temp,slow_temp);
	 msleep(3000);
	 MOTOR_STOP_P();
	 msleep(500);
	  if (stopcmd)
	 {
		  return;
	 }

	 MOTOR_RIGHT_P(fast_temp,slow_temp);
	 msleep(3000);
	 MOTOR_STOP_P();
	 msleep(500);
	 if (stopcmd)
	{
		 return;
	}
	 MOTOR_FORWARD_P(fast_temp,slow_temp);
	 msleep(600);
	 MOTOR_STOP_P();
	 msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	 MOTOR_BACK_P(fast_temp,slow_temp);
	 msleep(600);
	 MOTOR_STOP_P();
	 msleep(500);
	 if (stopcmd)
	{
		return;
	}
	 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
	 msleep(200);
	 MOTOR_H_STOP_P();
	 msleep(200);
	 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	 msleep(200);
	 MOTOR_H_STOP_P();
	 msleep(200);
	 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	 msleep(200);
	 MOTOR_H_STOP_P();
	 msleep(200);
	 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		 
	 msleep(200);
	 MOTOR_H_STOP_P();
	 msleep(200);
	 if (stopcmd)
	{
	    return;
	}
	 MOTOR_RIGHT_P(fast_temp,slow_temp);	 
	 msleep(3000);
	 MOTOR_STOP_P();
	 msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	 MOTOR_RIGHT_P(fast_temp,slow_temp);	 
	 msleep(1500);
	 MOTOR_STOP_P();
	 msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	 MOTOR_LEFT_P(fast_temp,slow_temp);
	 msleep(3000);
	 MOTOR_STOP_P();
	 msleep(500);
	 if (stopcmd)
	{
	    return;
	}
	 MOTOR_LEFT_P(fast_temp,slow_temp);
	 msleep(1500);
	 MOTOR_STOP_P();
	 msleep(500);
	 if (stopcmd)
	{
	    return;
	}
       for(i_idex=0;i_idex<=1;i_idex++)
      {
	  if (stopcmd)
	  {
	     break;
	  }
          MOTOR_FORWARD_P(fast_temp,slow_temp);
          msleep(600);
	   MOTOR_STOP_P();	   
	   msleep(500);
	 if (stopcmd)
         {
     	     return;
     	  }
	   MOTOR_BACK_P(fast_temp,slow_temp);
	   msleep(600);
	   MOTOR_STOP_P();
	   msleep(500);
	}
	  if (stopcmd)
	  {
	  return;
	   }
	  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
	  msleep(200);
	  MOTOR_V_STOP_P();
	  msleep(200);
	  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
	  msleep(200);
	  MOTOR_V_STOP_P();
	  msleep(200);
	 if (stopcmd)
	 {
		 return;
	  }
	 for(i_idex=0;i_idex<=5;i_idex++)
	 {
	  if (stopcmd)
     	  {
     	     break;
     	  }
	   MOTOR_RIGHT_P(fast_temp,slow_temp);
	   msleep(500);
	   MOTOR_STOP_P();
	   msleep(500);
	  }
	 if (stopcmd)
     	   {
     	       return;
     	   }
	 for(i_idex=0;i_idex<=5;i_idex++)
	 {
	 if (stopcmd)
          {
     	      break;
     	   }
	   MOTOR_LEFT_P(fast_temp,slow_temp);	   
	   msleep(500);
	   MOTOR_STOP_P();
	   msleep(500);
	 }
	 if (stopcmd)
     	   {
     	     return;
     	   }
	   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	   
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	 if (stopcmd)
	{
		return;
	}
	}
	  MOTOR_H_STOP_P();
	  MOTOR_V_STOP_P();
	  MOTOR_STOP_P();
 }

 
 /******************************************************/
 //第三套机器人舞蹈动作
 //fast=1,slow=0,    机器人处于快速工作状态
 //fast=0,slow=1，机器人处于慢速工作状态

void RobotDanceAction_31_P(unsigned int fast,unsigned int slow)
{
unsigned int i_idex;
unsigned int factor;//速度因子
unsigned fast_temp=0;
unsigned slow_temp=0;
fast_temp=fast;
slow_temp=slow;

if ((1==fast_temp)&&(0==slow_temp))
{
factor=0;
}

if ((1==slow_temp)&&(0==fast_temp))
{
factor=1;
}

#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)

if(!stopcmd)
{

	   MOTOR_RIGHT_P(fast_temp,slow_temp);
	   msleep(3000);
	   MOTOR_STOP_P();
	   msleep(500);
	   if (stopcmd)
     	    {
     	       return;
     	     }
	   MOTOR_RIGHT_P(fast_temp,slow_temp);
	   msleep(2000);
	   MOTOR_STOP_P();
	   msleep(500);
	   if (stopcmd)
     	    {
     	       return;
     	     }
	   MOTOR_LEFT_P(fast_temp,slow_temp);
	   msleep(3000);
	   MOTOR_STOP_P();
	   msleep(500);
	   if (stopcmd)
	    {
		   return;
	     }
	   MOTOR_LEFT_P(fast_temp,slow_temp);
	   msleep(2000);
	   MOTOR_STOP_P();
	   msleep(500);
	   if (stopcmd)
	    {
		   return;
	     }
	   
		MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		msleep(200);
              MOTOR_V_STOP_P();
		msleep(200);
              MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		msleep(200);
              MOTOR_V_STOP_P();
		msleep(200);
	if (stopcmd)
		{
			return;
		}

		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		msleep(200);
              MOTOR_V_STOP_P();
		msleep(200);
              MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		msleep(200);
              MOTOR_V_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		msleep(200);
              MOTOR_V_STOP_P();
		msleep(200);
              MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		msleep(200);
              MOTOR_V_STOP_P();
		msleep(200);
	if (stopcmd)
     	{
     	return;
     	}

		MOTOR_RIGHT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
       if (stopcmd)
     	{
     	return;
     	}
	   	MOTOR_RIGHT_P(fast_temp,slow_temp);
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
       if (stopcmd)
     	{
     	return;
     	}
	   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);    
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);    
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   if (stopcmd)
		   {
		   return;
		   }
		MOTOR_LEFT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
		MOTOR_LEFT_P(fast_temp,slow_temp);
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}	

		MOTOR_FORWARD_P(fast_temp,slow_temp);
		msleep(600);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
		MOTOR_BACK_P(fast_temp,slow_temp);
		msleep(600);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
			  
		 MOTOR_LEFT_P(fast_temp,slow_temp);
		 msleep(delaytime1000ms);
		 MOTOR_STOP_P();
		 msleep(500);
	if (stopcmd)
     	{
     	return;
     	}

		 MOTOR_RIGHT_P(fast_temp,slow_temp);
		 msleep(delaytime1000ms);
		 MOTOR_STOP_P();
		 msleep(500);
	if (stopcmd)
     	{
     	return;
     	}	 

		 MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		 msleep(200);
               MOTOR_V_STOP_P();
		msleep(200);
               MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		 msleep(200);
               MOTOR_V_STOP_P();
		msleep(200);
	if (stopcmd)
	{
	return;
	}

	 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
	 msleep(200);
        MOTOR_H_STOP_P();
	msleep(200);
        MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	 msleep(200);
        MOTOR_H_STOP_P();
	msleep(200);
	 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	 msleep(200);
        MOTOR_H_STOP_P();
	msleep(200);
        MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
	 msleep(200);
        MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
     	{
     	return;
     	}
}	
		MOTOR_H_STOP_P();
		MOTOR_V_STOP_P();
		MOTOR_STOP_P();
}
 
 /*****************************************************/
 //第四套机器人舞蹈动作
 //fast=1,slow=0,    机器人处于快速工作状态
 //fast=0,slow=1，机器人处于慢速工作状态

void RobotDanceAction_41_P(unsigned int fast,unsigned int slow)
{  

unsigned int i_idex;
unsigned int factor;//速度因子
unsigned fast_temp=0;
unsigned slow_temp=0;
fast_temp=fast;
slow_temp=slow;

if ((1==fast_temp)&&(0==slow_temp))
{
factor=0;
}

if ((1==slow_temp)&&(0==fast_temp))
{
factor=1;
}


#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)

 if(!stopcmd)
 {
           MOTOR_BACK_P(fast_temp,slow_temp);	    
	    msleep(600); 
	    MOTOR_STOP_P();
	    msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
           MOTOR_FORWARD_P(fast_temp,slow_temp);
           msleep(600);
	    MOTOR_STOP_P();
	    msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
           for (i_idex=0;i_idex<=1;i_idex++)
          {
             if (stopcmd)
             	{
             	break;
             	}
         	MOTOR_RIGHT_P(fast_temp,slow_temp);
		msleep(2000);
		MOTOR_STOP_P();
		msleep(500);
           }
	    if (stopcmd)
     	     {
     	      return;
     	     }	 
        for (i_idex=0;i_idex<=1;i_idex++)
          {
             if (stopcmd)
             	{
             	break;
             	}
         	MOTOR_LEFT_P(fast_temp,slow_temp);
		msleep(2000);
		MOTOR_STOP_P();
		msleep(500);
           }
	    if (stopcmd)
     	     {
     	      return;
     	     }	 
	  MOTOR_FORWARD_P(fast_temp,slow_temp);
	  msleep(600);
	  MOTOR_STOP_P();
	  msleep(500);
	 if (stopcmd)
     	{
     	return;
     	}
	  MOTOR_BACK_P(fast_temp,slow_temp);
	  msleep(600);
	  MOTOR_STOP_P();
	  msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
	  msleep(200);
         MOTOR_V_STOP_P();
	msleep(200);
         MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
	  msleep(200);
         MOTOR_V_STOP_P();
	msleep(200);
	  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
	  msleep(200);
	  MOTOR_V_STOP_P();
	 msleep(200);
	  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
	  msleep(200);
	  MOTOR_V_STOP_P();
	  msleep(200);
	if (stopcmd)
     	{
     	return;
     	}	  	  	
      
	  MOTOR_LEFT_P(fast_temp,slow_temp);
	  msleep(3000);
	  MOTOR_STOP_P();
	  msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	  MOTOR_LEFT_P(fast_temp,slow_temp);
	  msleep(1000);
	  MOTOR_STOP_P();
	  msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
         MOTOR_RIGHT_P(fast_temp,slow_temp);
	  msleep(3000);
	  MOTOR_STOP_P();
	  msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	  MOTOR_RIGHT_P(fast_temp,slow_temp);
	  msleep(1000);
	  MOTOR_STOP_P();
	  msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
     	{
     	return;
     	}
		MOTOR_FORWARD_P(fast_temp,slow_temp);
		msleep(600);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
		MOTOR_BACK_P(fast_temp,slow_temp);
		msleep(600);
		MOTOR_STOP_P();	
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}	  
	   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
	   msleep(200);
	   MOTOR_V_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
	   msleep(200);
	   MOTOR_V_STOP_P();
	   msleep(200);
	   if (stopcmd)
	  {
		   return;
	  }

	 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	 msleep(200);
        MOTOR_H_STOP_P();
	msleep(200);
        MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
	 msleep(200);
        MOTOR_H_STOP_P();
	msleep(200);
	 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
	 msleep(200);
        MOTOR_H_STOP_P();
	msleep(200);
        MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
	 msleep(200);
        MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
	{
	   return;
	}
    }
	   MOTOR_H_STOP_P();
	   MOTOR_V_STOP_P();
	   MOTOR_STOP_P();
}

//停止机器人舞蹈动作
 void RobotDance_Stop_P()
{
		MOTOR_STOP_P();
		MOTOR_H_STOP_P();
		MOTOR_V_STOP_P();
}

/************************************************************/
/************************************************************/
/************************************************************/
void RobotDanceAction_12_P(unsigned int fast,unsigned int slow)
{
       unsigned int i_idex;
	unsigned int factor;//速度因子
	
	unsigned fast_temp=0;
	unsigned slow_temp=0;
	fast_temp=fast;
	slow_temp=slow;
	
	
	if ((1==fast_temp)&&(0==slow_temp))
	{
	factor=0;
	}
	
	if ((1==slow_temp)&&(0==fast_temp))
	{
	factor=1;
	}
	
#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)
	
if (!stopcmd)
{
	
		MOTOR_FORWARD_P(fast_temp,slow_temp);
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}

		MOTOR_BACK_P(fast_temp,slow_temp);
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 if (stopcmd)
		{
			 return;
		}

		MOTOR_RIGHT_CLOCKDIR();
	       msleep(3000);
	       MOTOR_STOP_P();
		msleep(500);
             if (stopcmd)
     	      {
     	         return;
     	       }
			 MOTOR_RIGHT_ANTICLOCK();
			 msleep(3000);
			 MOTOR_STOP_P();
			 msleep(500);
			 if (stopcmd)
				 {
				  return;
				 }
		MOTOR_RIGHT_CLOCKDIR();
	       msleep(3000);
	       MOTOR_STOP_P();
	       msleep(500);
             if (stopcmd)
     	      {
     	         return;
     	       }

	      MOTOR_RIGHT_ANTICLOCK();
	      msleep(3000);
	      MOTOR_STOP_P();
	      msleep(500);
	      if (stopcmd)
     	      {
     	       return;
     	      }
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		if (stopcmd)
		{
			return;
		}
	
		MOTOR_LEFT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}

		MOTOR_LEFT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
     	{
     	return;
     	}
		MOTOR_RIGHT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
		 MOTOR_RIGHT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
		MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_V_STOP_P();
		msleep(200);
		MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_V_STOP_P();
		msleep(200);
		 if (stopcmd)
		{
			return;
		}

			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
		if (stopcmd)
		{
			return;
		}	
       for (i_idex=0;i_idex<=3;i_idex++)
       	{
       	if (stopcmd)
     	{
     	break;
     	}
       	MOTOR_LEFT_CLOCKDIR();
	msleep(1000);
	MOTOR_STOP_P();
	msleep(500);
       	}
       if (stopcmd)
     	{
     	return;
     	}
	   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);    
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);    
	   msleep(200);
	   MOTOR_H_STOP_P();
	   msleep(200);
	   if (stopcmd)
	   {
		   return;
	    }

	for(i_idex=0;i_idex<=3;i_idex++)
	{
	if (stopcmd)
     	{
     	break;
     	}
	MOTOR_LEFT_ANTICLOCK();
	msleep(1000);
	MOTOR_STOP_P();
	msleep(500);	
	}
	if (stopcmd)
     	{
     	return;
     	}
		
		MOTOR_FORWARD_P(fast_temp,slow_temp);
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
	if (stopcmd)
     	{
     	return;
     	}
		MOTOR_BACK_P(fast_temp,slow_temp);	
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
		{
			return;
		}
		MOTOR_H_STOP_P();
		MOTOR_V_STOP_P();
		MOTOR_STOP_P();
		}

}

/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_13_P(unsigned int fast,unsigned int slow)
{
	       unsigned int i_idex;
		unsigned int factor;//速度因子
		unsigned int i;
		
		unsigned fast_temp=0;
		unsigned slow_temp=0;
		fast_temp=fast;
		slow_temp=slow;
		
		
		if ((1==fast_temp)&&(0==slow_temp))
		{
		factor=0;
		}
		
		if ((1==slow_temp)&&(0==fast_temp))
		{
		factor=1;
		}
		
            #define delaytime400ms   200*(2-factor)
            #define delaytime600ms   300*(2-factor)
            #define delaytime800ms   400*(2-factor)
            #define delaytime1000ms 500*(2-factor)
		
	if (!stopcmd)
	{

		for(i=0;i<=3;i++)
		{
			if (stopcmd)
                     	{
                      	break;
                     	}
			MOTOR_FORWARD_P(fast_temp,slow_temp);
			msleep(500);
			MOTOR_STOP_P();
			msleep(500);
		}
	if (stopcmd)
     	{
     	return;
     	}
		for(i=0;i<=3;i++)
		{
			if (stopcmd)
               	{
     	                 break;
     	               }
			MOTOR_BACK_P(fast_temp,slow_temp);
			msleep(500);
			MOTOR_STOP_P();
			msleep(500);
		}
	if (stopcmd)
     	{
     	return;
     	}
			MOTOR_LEFT_CLOCKDIR();
			msleep(3000);
			MOTOR_STOP_P();
			msleep(500);
			if (stopcmd)
			{
			     return;
			}
			MOTOR_LEFT_CLOCKDIR();
			msleep(2000);
			MOTOR_STOP_P();
			msleep(500);
			if (stopcmd)
			{
			     return;
			}
			  MOTOR_RIGHT_ANTICLOCK();
			  msleep(3000);
			  MOTOR_STOP_P();
			  msleep(500);
			  if (stopcmd)
			 {
			      return;
			 }
			  MOTOR_RIGHT_ANTICLOCK();
			  msleep(2000);
			  MOTOR_STOP_P();
			  msleep(500);
			  if (stopcmd)
			 {
			      return;
			 }
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			if (stopcmd)
			{
				return;
			}
		
			MOTOR_LEFT_P(fast_temp,slow_temp);
			msleep(3000);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
			MOTOR_LEFT_P(fast_temp,slow_temp);
			msleep(2000);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
			MOTOR_RIGHT_P(fast_temp,slow_temp);
			msleep(3000);
			MOTOR_STOP_P();
			msleep(500);
			 if (stopcmd)
			{
				return;
			}
			 MOTOR_RIGHT_P(fast_temp,slow_temp);
			msleep(2000);
			MOTOR_STOP_P();
			msleep(500);
			 if (stopcmd)
			{
				return;
			}

			MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_V_STOP_P();
			msleep(200);
			MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_V_STOP_P();
			msleep(200);
			 if (stopcmd)
			{
				return;
			}
			 
				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 	
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 	
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);

			if (stopcmd)
			{
				return;
			}
			for (i_idex=0;i_idex<=3;i_idex++)
			{
			 if (stopcmd)
			{
			     break;
			}
			MOTOR_RIGHT_P(fast_temp,slow_temp);
			msleep(500);
			MOTOR_STOP_P();
			msleep(500);
		       if (stopcmd)
     	              {
     	                  return;
     	              }
			MOTOR_LEFT_P(fast_temp,slow_temp);
			msleep(500);
			MOTOR_STOP_P();
			msleep(500);
			}
			if (stopcmd)
			{
				return;
			}

		MOTOR_LEFT_CLOCKDIR();
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
		MOTOR_LEFT_CLOCKDIR();
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
		MOTOR_LEFT_ANTICLOCK();
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
			{
			return;
			}
		MOTOR_LEFT_ANTICLOCK();
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
			{
			return;
			}
		
			MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_V_STOP_P();
			msleep(200);
			MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			msleep(100);
			MOTOR_V_STOP_P();
			msleep(200);
			if (stopcmd)
			{
				return;
			}
		
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			if (stopcmd)
			{
				return;
			}
			MOTOR_H_STOP_P();
			MOTOR_V_STOP_P();
			MOTOR_STOP_P();
			}


}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_14_P(unsigned int fast,unsigned int slow)
{
	         unsigned int i_idex;
		  unsigned int factor;//速度因子
		  unsigned int i;
		  
		  unsigned fast_temp=0;
		  unsigned slow_temp=0;
		  fast_temp=fast;
		  slow_temp=slow;
		  
		  
		  if ((1==fast_temp)&&(0==slow_temp))
		  {
		  factor=0;
		  }
		  
		  if ((1==slow_temp)&&(0==fast_temp))
		  {
		  factor=1;
		  }
		  
		  #define delaytime400ms   200*(2-factor)
		  #define delaytime600ms   300*(2-factor)
		  #define delaytime800ms   400*(2-factor)
		  #define delaytime1000ms 500*(2-factor)
		  
	if (!stopcmd)
	{
	      for(i_idex=0;i_idex<=1;i_idex++)
	      	{
	      	if (stopcmd)
     	        {
     	               break;
     	        }
	      	  MOTOR_LEFT_ANTICLOCK();
		  msleep(2000);
		  MOTOR_STOP_P();
		  msleep(500);

	      	}
		  if (stopcmd)
     	        {
     	               return;
     	        }

		for(i_idex=0;i_idex<=1;i_idex++)
	      	{
	      	if (stopcmd)
     	        {
     	               break;
     	        }
			MOTOR_RIGHT_ANTICLOCK();
			msleep(2000);
			MOTOR_STOP_P();
			msleep(500);
	      	}
		if (stopcmd)
     	        {
     	               return;
     	        }
		  for(i=0;i<=1;i++)
		  {
		     if (stopcmd)
     	              {
     	                   break;
     	              }
			  MOTOR_FORWARD_P(fast_temp,slow_temp);
			  msleep(500);
			  MOTOR_STOP_P();
			  msleep(500);
		  }
		  if (stopcmd)
		  {
			 return;
		  }
		  for(i=0;i<=1;i++)
		  {
		      if (stopcmd)
     	               {
     	                       break;
                      }
			  MOTOR_BACK_P(fast_temp,slow_temp);
			  msleep(500);
			  MOTOR_STOP_P();
			  msleep(500);
		  }
		  if (stopcmd)
		  {
			 return;
		  }
			  MOTOR_FORWARD_P(fast_temp,slow_temp);
			  msleep(1500);
			  MOTOR_STOP_P();
			  msleep(500);
			   if (stopcmd)
     	                  {
     	                         return;
     	                  }
			  MOTOR_BACK_P(fast_temp,slow_temp);
			  msleep(1500);
			  MOTOR_STOP_P();
			  msleep(500);
			   if (stopcmd)
			  {
				  return;
			  }
			   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);    
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);    
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   if (stopcmd)
			   {
				   return;
			   }
			  MOTOR_LEFT_CLOCKDIR();
			  msleep(3000);
			  MOTOR_STOP_P();
			  msleep(500);
			  if (stopcmd)
			  {
				   return;
			  }
			  MOTOR_LEFT_CLOCKDIR();
			  msleep(1000);
			  MOTOR_STOP_P();
			  msleep(500);
			  if (stopcmd)
			  {
				   return;
			  }
				MOTOR_RIGHT_ANTICLOCK();
				msleep(3000);
				MOTOR_STOP_P();
				msleep(500);
				if (stopcmd)
			   {
					return;
			   }
				MOTOR_RIGHT_ANTICLOCK();
				msleep(1000);
				MOTOR_STOP_P();
				msleep(500);
				if (stopcmd)
			   {
					return;
			   }
			  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
			  msleep(200);
			  MOTOR_H_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			  msleep(200);
			  MOTOR_H_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			  msleep(200);
			  MOTOR_H_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
			  msleep(200);
			  MOTOR_H_STOP_P();
			  msleep(200);
			  if (stopcmd)
			  {
				  return;
			  }
		  
			  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			   if (stopcmd)
			  {
				  return;
			  }

				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	  
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		  
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		  
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	  
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);

			  if (stopcmd)
			  {
				  return;
			  }  

			  for(i_idex=0;i_idex<=3;i_idex++)
			  {
				  if (stopcmd)
				 {
					break;
				 }
			  MOTOR_RIGHT_P(fast_temp,slow_temp);
			  msleep(1000);
			  MOTOR_STOP_P();
			  msleep(500);
			  if (stopcmd)
     	                    {
     	                              return;
     	                    }
			  MOTOR_LEFT_P(fast_temp,slow_temp);
			  msleep(1000);
			  MOTOR_STOP_P();
			  msleep(500);
			  }
			  if (stopcmd)
			  {
				  return;
			  }
		  MOTOR_RIGHT_CLOCKDIR();
		  msleep(3000);
		  MOTOR_STOP_P();
		  msleep(500);
		   if (stopcmd)
		   {
			  return;
		   }
		  MOTOR_RIGHT_ANTICLOCK();
		  msleep(3000);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
		   {
			  return;
		   }
		  
			  MOTOR_H_STOP_P();
			  MOTOR_V_STOP_P();
			  MOTOR_STOP_P();
		}

}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_15_P(unsigned int fast,unsigned int slow)
{
	unsigned int i_idex;
	unsigned int factor;//速度因子
	unsigned int i;
	
	unsigned fast_temp=0;
	unsigned slow_temp=0;
	fast_temp=fast;
	slow_temp=slow;
	
	
	if ((1==fast_temp)&&(0==slow_temp))
	{
	factor=0;
	}
	
	if ((1==slow_temp)&&(0==fast_temp))
	{
	factor=1;
	}
	
      #define delaytime400ms	 200*(2-factor)
      #define delaytime600ms	 300*(2-factor)
      #define delaytime800ms	 400*(2-factor)
      #define delaytime1000ms  500*(2-factor)
	
if (!stopcmd)
{
	
	       MOTOR_RIGHT_ANTICLOCK();
	       msleep(2500);
	       MOTOR_STOP_P();
	       msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }
	       MOTOR_LEFT_ANTICLOCK();
	       msleep(2500);
	       MOTOR_STOP_P();
	       msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 if (stopcmd)
		{
			 return;
		}
		MOTOR_FORWARD_P(fast_temp,slow_temp);
		msleep(500);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }

		MOTOR_BACK_P(fast_temp,slow_temp);
		msleep(500);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
	
		MOTOR_FORWARD_P(fast_temp,slow_temp);
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
		MOTOR_BACK_P(fast_temp,slow_temp);
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
	msleep(200);
	MOTOR_H_STOP_P();
	msleep(200);
	if (stopcmd)
     	{
     	return;
     	}
		MOTOR_LEFT_CLOCKDIR();
		msleep(1000);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
		{
			 return;
		}
		  MOTOR_RIGHT_ANTICLOCK();
		  msleep(1000);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
		 {
			  return;
		 }
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		if (stopcmd)
		{
			return;
		}
	
		MOTOR_LEFT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
		MOTOR_LEFT_P(fast_temp,slow_temp);
		msleep(1500);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
		MOTOR_RIGHT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
		 MOTOR_RIGHT_P(fast_temp,slow_temp);
		msleep(1500);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
		MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_V_STOP_P();
		msleep(200);
		MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_V_STOP_P();
		msleep(200);
		 if (stopcmd)
		{
			return;
		}
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
		if (stopcmd)
		{
			return;
		}	
		MOTOR_RIGHT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
		MOTOR_LEFT_P(fast_temp,slow_temp);
		msleep(3000);
		MOTOR_STOP_P();
		msleep(500);
		 if (stopcmd)
		{
			return;
		}
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 if (stopcmd)
		{
			 return;
		}

	MOTOR_RIGHT_CLOCKDIR();
	msleep(3000);
	MOTOR_STOP_P();
	msleep(500);
	 if (stopcmd)
	{
		return;
	}
	MOTOR_RIGHT_ANTICLOCK();
	msleep(3000);
	MOTOR_STOP_P();
	msleep(500);
	if (stopcmd)
	{
		return;
	}
	
		MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_V_STOP_P();
		msleep(200);
		MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_V_STOP_P();
		msleep(200);
		if (stopcmd)
		{
			return;
		}

	for(i=0;i<=2;i++)
	{
		   if(stopcmd)
                 {
                     break;
		   }
		MOTOR_FORWARD_P(fast_temp,slow_temp);
		msleep(500);
		MOTOR_STOP_P();
		msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }

		MOTOR_BACK_P(fast_temp,slow_temp);	
		msleep(500);
		MOTOR_STOP_P();
		msleep(500);
	}
		
		if (stopcmd)
		{
			return;
		}
	
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		if (stopcmd)
		{
			return;
		}
		MOTOR_H_STOP_P();
		MOTOR_V_STOP_P();
		MOTOR_STOP_P();
		}

}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_22_P(unsigned int fast,unsigned int slow)
{
	unsigned int i_idex;
	unsigned int factor;//速度因子
	unsigned fast_temp=0;
	unsigned slow_temp=0;
	fast_temp=fast;
	slow_temp=slow;
	
	if ((1==fast_temp)&&(0==slow_temp))
	{
	factor=0;
	}
	
	if ((1==slow_temp)&&(0==fast_temp))
	{
	factor=1;
	}
	
	unsigned int result;
       #define delaytime400ms   200*(2-factor)
       #define delaytime600ms   300*(2-factor)
       #define delaytime800ms   400*(2-factor)
       #define delaytime1000ms 500*(2-factor)
	 
	 if (!stopcmd)
	 {
		 MOTOR_LEFT_P(fast_temp,slow_temp);
		 msleep(3000);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }
		 MOTOR_LEFT_P(fast_temp,slow_temp);
		 msleep(3000);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }
		 MOTOR_RIGHT_P(fast_temp,slow_temp);
		 msleep(3000);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
		{
			 return;
		}
		 MOTOR_RIGHT_P(fast_temp,slow_temp);
		 msleep(3000);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
		{
			 return;
		}
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 if (stopcmd)
		{
			 return;
		}
		 MOTOR_FORWARD_P(fast_temp,slow_temp);
		 msleep(1500);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }

		 MOTOR_BACK_P(fast_temp,slow_temp);
		 msleep(1500);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
		{
			 return;
		}
		  MOTOR_LEFT_ANTICLOCK();
		  msleep(3000);
		  MOTOR_STOP_P();
		  msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 if (stopcmd)
		{
			 return;
		}
		  MOTOR_RIGHT_ANTICLOCK();
		  msleep(3000);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }
		  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);		 
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		 if (stopcmd)
			 {
			 return;
			 }

	  for(i_idex=0;i_idex<=2;i_idex++)
	   {
		  if (stopcmd)
		{
			 break;
		}

		   MOTOR_FORWARD_P(fast_temp,slow_temp);
		    msleep(1000);
		   MOTOR_STOP_P();	   
		   msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }
		   MOTOR_BACK_P(fast_temp,slow_temp);
		   msleep(1000);
		   MOTOR_STOP_P();
		   msleep(500);
		 if (stopcmd)
		  {
		  return;
		   }
	 
		   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		 if (stopcmd)
		{
			 return;
		}

		   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_H_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_H_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_H_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_H_STOP_P();
		   msleep(200);
		  if (stopcmd)
		  {
			 return;
		  }
		  MOTOR_RIGHT_P(fast_temp,slow_temp);
		  msleep(1500);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }
		  MOTOR_LEFT_P(fast_temp,slow_temp);	   
		  msleep(1500);
		  MOTOR_STOP_P();
		  msleep(500);
		}
	  if (stopcmd)
	  {
		 return;
	  }

	}
			MOTOR_H_STOP_P();
			MOTOR_V_STOP_P();
			MOTOR_STOP_P();

}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_23_P(unsigned int fast,unsigned int slow)
{

	unsigned int i_idex;
	unsigned int factor;//速度因子
	unsigned int i;
	unsigned fast_temp=0;
	unsigned slow_temp=0;
	fast_temp=fast;
	slow_temp=slow;
	
	if ((1==fast_temp)&&(0==slow_temp))
	{
	factor=0;
	}
	
	if ((1==slow_temp)&&(0==fast_temp))
	{
	factor=1;
	}
	
	unsigned int result;
      #define delaytime400ms	200*(2-factor)
      #define delaytime600ms	300*(2-factor)
      #define delaytime800ms	400*(2-factor)
      #define delaytime1000ms 500*(2-factor)
	 
 if (!stopcmd)
{
	 for(i=0;i<=1;i++)
	 {
	        if (stopcmd)
     	        {
     	               break;
     	        }
		 MOTOR_LEFT_P(fast_temp,slow_temp);
		 msleep(3000);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }
		 MOTOR_RIGHT_P(fast_temp,slow_temp);
		 msleep(3000);
		 MOTOR_STOP_P();
		 msleep(500);
	 }
		 if (stopcmd)
		{
			 return;
		}
		 MOTOR_FORWARD_P(fast_temp,slow_temp);
		 msleep(1500);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
     	        {
     	               return;
     	        }
		 MOTOR_BACK_P(fast_temp,slow_temp);
		 msleep(1500);
		 MOTOR_STOP_P();
		 msleep(500);
		 if (stopcmd)
		{
			 return;
		}
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 if (stopcmd)
			 {
			 return;
			 }
		  MOTOR_LEFT_CLOCKDIR();
		  msleep(3000);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }
		  MOTOR_RIGHT_CLOCKDIR();
		  msleep(3000);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }

	  for(i_idex=0;i_idex<=2;i_idex++)
	   {
		  if (stopcmd)
		{
			 break;
		}

		   MOTOR_FORWARD_P(fast_temp,slow_temp);
		    msleep(800);
		   MOTOR_STOP_P();	   
		   msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }
		   MOTOR_BACK_P(fast_temp,slow_temp);
		   msleep(800);
		   MOTOR_STOP_P();
		   msleep(500);
		 if (stopcmd)
		  {
		  return;
		   }
		   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		 if (stopcmd)
		{
			 return;
		}

		   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_H_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_H_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_H_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_H_STOP_P();
		   msleep(200);
		  if (stopcmd)
		  {
			 return;
		   }

		  MOTOR_LEFT_CLOCKDIR();
		  msleep(1000);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }
		  MOTOR_RIGHT_ANTICLOCK();
		  msleep(1000);
		  MOTOR_STOP_P();
		  msleep(500);
		}
	       if (stopcmd)
     	        {
     	               return;
     	        }

	}
			MOTOR_H_STOP_P();
			MOTOR_V_STOP_P();
			MOTOR_STOP_P();

}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_24_P(unsigned int fast,unsigned int slow)
{
	       unsigned int i_idex;
		unsigned int factor;//速度因子
		unsigned int i;
		unsigned fast_temp=0;
		unsigned slow_temp=0;
		fast_temp=fast;
		slow_temp=slow;
		
		if ((1==fast_temp)&&(0==slow_temp))
		{
		factor=0;
		}
		
		if ((1==slow_temp)&&(0==fast_temp))
		{
		factor=1;
		}
		
		unsigned int result;
              #define delaytime400ms	200*(2-factor)
              #define delaytime600ms	300*(2-factor)
              #define delaytime800ms	400*(2-factor)
              #define delaytime1000ms 500*(2-factor)
		 
	if (!stopcmd)
	{
		 for(i=0;i<=1;i++)
		 {
		       if (stopcmd)
     	            {
     	               break;
     	            }
			 MOTOR_LEFT_P(fast_temp,slow_temp);
			 msleep(2000);
			 MOTOR_STOP_P();
			 msleep(500);
			if (stopcmd)
     	                {
     	                     return;
     	                }
			 MOTOR_RIGHT_P(fast_temp,slow_temp);
			 msleep(2000);
			 MOTOR_STOP_P();
			 msleep(500);
		 }
			 if (stopcmd)
			 {
				 return;
			 }
			 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
			 msleep(200);
			 MOTOR_H_STOP_P();
			 msleep(200);
			 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			 msleep(200);
			 MOTOR_H_STOP_P();
			 msleep(200);
			 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			 msleep(200);
			 MOTOR_H_STOP_P();
			 msleep(200);
			 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
			 msleep(200);
			 MOTOR_H_STOP_P();
			 msleep(200);
			 if (stopcmd)
			{
				 return;
			}
			 MOTOR_FORWARD_P(fast_temp,slow_temp);
			 msleep(1200);
			 MOTOR_STOP_P();
			 msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
			 MOTOR_BACK_P(fast_temp,slow_temp);
			 msleep(1200);
			 MOTOR_STOP_P();
			 msleep(500);
			 if (stopcmd)
			 {
				 return;
			 }
			  MOTOR_LEFT_CLOCKDIR();
			  msleep(1500);
			  MOTOR_STOP_P();
			  msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
			  MOTOR_RIGHT_CLOCKDIR();
			  msleep(1500);
			  MOTOR_STOP_P();
			  msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
               for(i=0;i<=5;i++)
               {
		         if (stopcmd)
			  {
				break;
			   }
               	 MOTOR_RIGHT_P(fast_temp,slow_temp);	 
			 msleep(1000);
			 MOTOR_STOP_P();
			 msleep(500);
               }
		       if (stopcmd)
     	              {
     	                      return;
     	              }
			for(i=0;i<=5;i++)
			{
			     if (stopcmd)
				{
				  break;
				 }
			   MOTOR_LEFT_P(fast_temp,slow_temp);	   
			   msleep(1000);
			   MOTOR_STOP_P();
			   msleep(500);
			}
			 if (stopcmd)
			{
				return;
			}
			   MOTOR_FORWARD_P(fast_temp,slow_temp);
			   msleep(800);
			   MOTOR_STOP_P();	   
			   msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
			   MOTOR_BACK_P(fast_temp,slow_temp);
			   msleep(800);
			   MOTOR_STOP_P();
			   msleep(500);
			 if (stopcmd)
			  {
			  return;
			   }
		 
			   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_V_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_V_STOP_P();
			   msleep(200);
			 if (stopcmd)
			{
				 return;
			}
			
			   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			  if (stopcmd)
			{
				 return;
			}

			  MOTOR_RIGHT_P(fast_temp,slow_temp);
			  msleep(3000);
			  MOTOR_STOP_P();
			  msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
			  MOTOR_RIGHT_P(fast_temp,slow_temp);
			  msleep(1000);
			  MOTOR_STOP_P();
			  msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
			  MOTOR_LEFT_P(fast_temp,slow_temp);	   
			  msleep(3000);
			  MOTOR_STOP_P();
			  msleep(500);
			 if (stopcmd)
			{
				 return;
			}
			 MOTOR_LEFT_P(fast_temp,slow_temp);	   
			  msleep(1000);
			  MOTOR_STOP_P();
			  msleep(500);
			 if (stopcmd)
			{
				 return;
			}
			 
			  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			 if (stopcmd)
			{
				 return;
			}
			   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);    
			   msleep(200);
			   MOTOR_H_STOP_P();
			   msleep(200);
			 if (stopcmd)
			{
				 return;
			}
		 	}
				 
				MOTOR_H_STOP_P();
				MOTOR_V_STOP_P();
				MOTOR_STOP_P();

}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_25_P(unsigned int fast,unsigned int slow)
{
			unsigned int i_idex;
			unsigned int factor;//速度因子
			unsigned int i;
			unsigned fast_temp=0;
			unsigned slow_temp=0;
			fast_temp=fast;
			slow_temp=slow;
			
			if ((1==fast_temp)&&(0==slow_temp))
			{
			factor=0;
			}
			
			if ((1==slow_temp)&&(0==fast_temp))
			{
			factor=1;
			}
			
		unsigned int result;
              #define delaytime400ms	200*(2-factor)
              #define delaytime600ms	300*(2-factor)
              #define delaytime800ms	400*(2-factor)
              #define delaytime1000ms 500*(2-factor)
			 
		if (!stopcmd)
		{
			 for(i=0;i<=2;i++)
			 {
			    if (stopcmd)
     	                    {
     	                          break;
     	                    }
				 MOTOR_LEFT_P(fast_temp,slow_temp);
				 msleep(1000);
				 MOTOR_STOP_P();
				 msleep(500);
			    if (stopcmd)
     	                   {
     	                        return;
     	                   }
				 MOTOR_RIGHT_P(fast_temp,slow_temp);
				 msleep(1000);
				 MOTOR_STOP_P();
				 msleep(500);
			 }
				 if (stopcmd)
					 {
					 return;
					 }
				 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 if (stopcmd)
					 {
					 return;
					 }
				 MOTOR_BACK_P(fast_temp,slow_temp);
				 msleep(1000);
				 MOTOR_STOP_P();
				 msleep(500);
				 if (stopcmd)
     	                       {
     	                           return;
     	                       }
				 MOTOR_FORWARD_P(fast_temp,slow_temp);
				 msleep(1000);
				 MOTOR_STOP_P();
				 msleep(500);
				 if (stopcmd)
				{
					 return;
				}
				 for(i=0;i<=1;i++)
				 {
				 	if (stopcmd)
     	                            {
     	                                 return;
     	                             }
				  MOTOR_LEFT_CLOCKDIR();
				  msleep(3000);
				  MOTOR_STOP_P();
				  msleep(500);
				  if (stopcmd)
      	                         {
      	                               return;
      	                         }
				  MOTOR_RIGHT_CLOCKDIR();
				  msleep(3000);
				  MOTOR_STOP_P();
				  msleep(500);
				 }
				 
				 if (stopcmd)
				{
					return;
				}
				 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 if (stopcmd)
				{
					 return;
				}
				 MOTOR_RIGHT_P(fast_temp,slow_temp);	 
				 msleep(3000);
				 MOTOR_STOP_P();
				 msleep(500);
				 if (stopcmd)
     	                      {
     	                              return;
     	                       }
				 MOTOR_RIGHT_P(fast_temp,slow_temp);	 
				 msleep(1500);
				 MOTOR_STOP_P();
				 msleep(500);
				 if (stopcmd)
     	                      {
     	                              return;
     	                       }
				 MOTOR_LEFT_P(fast_temp,slow_temp);
				 msleep(3000);
				 MOTOR_STOP_P();
				 msleep(500);
				 if (stopcmd)
				{
					return;
				}
				 MOTOR_LEFT_P(fast_temp,slow_temp);
				 msleep(1500);
				 MOTOR_STOP_P();
				 msleep(500);
				 if (stopcmd)
				{
					return;
				}
				   MOTOR_FORWARD_P(fast_temp,slow_temp);
				   msleep(1000);
				   MOTOR_STOP_P();	   
				   msleep(500);
				 if (stopcmd)
     	                         {
     	                               return;
     	                         }
				   MOTOR_BACK_P(fast_temp,slow_temp);
				   msleep(1000);
				   MOTOR_STOP_P();
				   msleep(500);
				 if (stopcmd)
				  {
				  return;
				   }
				   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_V_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_V_STOP_P();
				   msleep(200);
				 if (stopcmd)
					 {
					 return;
					 }
				   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_H_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_H_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_H_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_H_STOP_P();
				   msleep(200);
				  if (stopcmd)
				  {
					 return;
				   }
		               for(i=0;i<=1;i++)
		               {

				   if (stopcmd)
      	                            {
      	                                  break;
      	                             }
				     MOTOR_LEFT_CLOCKDIR();
				     msleep(2000);
				     MOTOR_STOP_P();
				     msleep(500);
				    if (stopcmd)
     	                           {
     	                                 return;
     	                           }
				     MOTOR_RIGHT_ANTICLOCK();
				     msleep(2000);
				     MOTOR_STOP_P();
				     msleep(500);
		               }
					   if (stopcmd)
					   {
						  return;
					   }

				  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				 if (stopcmd)
				 {
					 return;
				 }
				   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_H_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_H_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_H_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);    
				   msleep(200);
				   MOTOR_H_STOP_P();
				   msleep(200);
				 if (stopcmd)
				{
					 return;
				}
					 
					MOTOR_H_STOP_P();
					MOTOR_V_STOP_P();
					MOTOR_STOP_P();
	
             }
}

/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_32_P(unsigned int fast,unsigned int slow)
{
	unsigned int i_idex;
	unsigned int i;
	unsigned int factor;//速度因子
	unsigned fast_temp=0;
	unsigned slow_temp=0;
	fast_temp=fast;
	slow_temp=slow;
	
	if ((1==fast_temp)&&(0==slow_temp))
	{
	factor=0;
	}
	
	if ((1==slow_temp)&&(0==fast_temp))
	{
	factor=1;
	}
	
#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)
		
	if(!stopcmd)
	{
	
		   MOTOR_RIGHT_P(fast_temp,slow_temp);
		   msleep(2500);
		   MOTOR_STOP_P();
		   msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }
		   MOTOR_RIGHT_P(fast_temp,slow_temp);
		   msleep(2500);
		   MOTOR_STOP_P();
		   msleep(500);
		  if (stopcmd)
     	        {
     	               return;
     	        }
		  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  if (stopcmd)
			  {
			  return;
			  }
		   MOTOR_LEFT_P(fast_temp,slow_temp);
		   msleep(2500);
		   MOTOR_STOP_P();
		   msleep(500);
		   if (stopcmd)
		   {
			   return;
		    }
		   MOTOR_LEFT_P(fast_temp,slow_temp);
		   msleep(2500);
		   MOTOR_STOP_P();
		   msleep(500);
		   if (stopcmd)
		   {
			   return;
		    }
		   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		  if (stopcmd)
		  {
			return;
		  }
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
		if (stopcmd)
		{
			return;
		}

		for(i=0;i<=3;i++)
		{
			if (stopcmd)
       	               {
       	                    break;
       	               }
			MOTOR_FORWARD_P(fast_temp,slow_temp);
			msleep(400);
			MOTOR_STOP_P();
			msleep(500);
		}
		if (stopcmd)
		{
		   return;
		}

		for(i=0;i<=3;i++)
			{
			   if (stopcmd)
     	                   {
     	                         break;
     	                    }
			MOTOR_BACK_P(fast_temp,slow_temp);
			msleep(400);
			MOTOR_STOP_P();
			msleep(500);
			}
		if (stopcmd)
		{
		return;
		}
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			if (stopcmd)
				{
				return;
				}
			MOTOR_RIGHT_P(fast_temp,slow_temp);
			msleep(2000);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
     	        {
     	               return;
     	        }
			MOTOR_LEFT_P(fast_temp,slow_temp);
			msleep(2000);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
		{
			return;
		}
		  
			MOTOR_FORWARD_P(fast_temp,slow_temp);
			msleep(1500);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
			{
			return;
			}
			MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_V_STOP_P();
			msleep(200);
			MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_V_STOP_P();
			msleep(200);
		if (stopcmd)
			{
			return;
			}
			MOTOR_BACK_P(fast_temp,slow_temp);
			msleep(1500);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
			{
			return;
			}
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		       msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		       msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
		       MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		       msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		       msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
		if (stopcmd)
			{
			return;
			}

			 MOTOR_RIGHT_P(fast_temp,slow_temp);
			 msleep(3000);
			 MOTOR_STOP_P();
			 msleep(500);
		if (stopcmd)
			{
			return;
			}
			 MOTOR_RIGHT_P(fast_temp,slow_temp);
			 msleep(2000);
			 MOTOR_STOP_P();
			 msleep(500);
		if (stopcmd)
			{
			return;
			}
			 MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			 msleep(200);
			 MOTOR_V_STOP_P();
			 msleep(200);
			 MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			 msleep(200);
			 MOTOR_V_STOP_P();
			 msleep(200);
		if (stopcmd)
		{
		return;
		}
			 MOTOR_LEFT_P(fast_temp,slow_temp);
			 msleep(3000);
			 MOTOR_STOP_P();
			 msleep(500);
		if (stopcmd)
		{
			return;
		}
			MOTOR_LEFT_P(fast_temp,slow_temp);
			 msleep(2000);
			 MOTOR_STOP_P();
			 msleep(500);
		if (stopcmd)
		{
			return;
		}	
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		 msleep(200);
		 MOTOR_H_STOP_P();
		 msleep(200);
		if (stopcmd)
		{
			return;
		}
	}	
			MOTOR_H_STOP_P();
			MOTOR_V_STOP_P();
			MOTOR_STOP_P();

}

/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_33_P(unsigned int fast,unsigned int slow)
{
		unsigned int i_idex;
		unsigned int i;
		unsigned int factor;//速度因子
		unsigned fast_temp=0;
		unsigned slow_temp=0;
		fast_temp=fast;
		slow_temp=slow;
		
		if ((1==fast_temp)&&(0==slow_temp))
		{
		factor=0;
		}
		
		if ((1==slow_temp)&&(0==fast_temp))
		{
		factor=1;
		}
		
#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)
			
		if(!stopcmd)
		{
		      for(i=0;i<=1;i++)
		      	{
		      	   if (stopcmd)
     	                  {
     	                       break;
               	    }
			   MOTOR_RIGHT_P(fast_temp,slow_temp);
			   msleep(3000);
			   MOTOR_STOP_P();
			   msleep(500);
			   if (stopcmd)
     	                 {
     	                       return;
     	                 }
			   MOTOR_LEFT_P(fast_temp,slow_temp);
			   msleep(3000);
			   MOTOR_STOP_P();
			   msleep(500);
		      	}
			   if (stopcmd)
			   {
				   return;
			   }
			   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_V_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_V_STOP_P();
			   msleep(200);
			if (stopcmd)
			{
				return;
			}
				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
			if (stopcmd)
			{
				return;
			}

				MOTOR_LEFT_P(fast_temp,slow_temp);
				msleep(2500);
				MOTOR_STOP_P();
				msleep(500);
			  if (stopcmd)
     	                  {
                 	             return;
                     	    }
				MOTOR_RIGHT_P(fast_temp,slow_temp);
				msleep(2500);
				MOTOR_STOP_P();
				msleep(500);
			if (stopcmd)
				{
				return;
				}
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			if (stopcmd)
				{
				return;
				}
	
			for(i=0;i<=1;i++)
			{
			     if (stopcmd)
     	                   {
     	                        break;
     	                   }
				MOTOR_FORWARD_P(fast_temp,slow_temp);
				msleep(500);
				MOTOR_STOP_P();
				msleep(500);
			}
			if (stopcmd)
				{
				return;
				}
			   for(i=0;i<=1;i++)
				{
				    if (stopcmd)
                	        {
                	               break;
                	        }
				MOTOR_BACK_P(fast_temp,slow_temp);
				msleep(500);
				MOTOR_STOP_P();
				msleep(500);
				}
			if (stopcmd)
				{
				return;
				}
		  
				MOTOR_FORWARD_P(fast_temp,slow_temp);
				msleep(2000);
				MOTOR_STOP_P();
				msleep(500);
			if (stopcmd)
			{
				return;
			}
				MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_V_STOP_P();
				msleep(200);
				MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_V_STOP_P();
				msleep(200);
			if (stopcmd)
			{
				return;
			}
				MOTOR_BACK_P(fast_temp,slow_temp);
				msleep(2000);
				MOTOR_STOP_P();
				msleep(500);
			if (stopcmd)
			{
				return;
			}
				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
			if (stopcmd)
			{
				return;
			}
				
				 MOTOR_LEFT_P(fast_temp,slow_temp);
				 msleep(3000);
				 MOTOR_STOP_P();
				 msleep(500);
			if (stopcmd)
				{
				return;
				}
				 MOTOR_LEFT_P(fast_temp,slow_temp);
				 msleep(1500);
				 MOTOR_STOP_P();
				 msleep(500);
			if (stopcmd)
				{
				return;
				}
				 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_V_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_V_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_V_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_V_STOP_P();
				 msleep(200);
			if (stopcmd)
			{
				return;
			}
				 MOTOR_RIGHT_P(fast_temp,slow_temp);
				 msleep(3000);
				 MOTOR_STOP_P();
				 msleep(500);
			if (stopcmd)
			{
				return;
			}
				MOTOR_RIGHT_P(fast_temp,slow_temp);
				 msleep(1500);
				 MOTOR_STOP_P();
				 msleep(500);
			if (stopcmd)
			{
				return;
			}	

				 MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_V_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_V_STOP_P();
				 msleep(200);
			if (stopcmd)
			{
			 return;
			} 

			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			msleep(200);
			MOTOR_H_STOP_P();
			msleep(200);
			if (stopcmd)
				{
				return;
				}
		}	
				MOTOR_H_STOP_P();
				MOTOR_V_STOP_P();
				MOTOR_STOP_P();
}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_34_P(unsigned int fast,unsigned int slow)
{
			unsigned int i_idex;
			unsigned int i;
			unsigned int factor;//速度因子
			unsigned fast_temp=0;
			unsigned slow_temp=0;
			fast_temp=fast;
			slow_temp=slow;
			
			if ((1==fast_temp)&&(0==slow_temp))
			{
			factor=0;
			}
			
			if ((1==slow_temp)&&(0==fast_temp))
			{
			factor=1;
			}
			
                 #define delaytime400ms   200*(2-factor)
                 #define delaytime600ms   300*(2-factor)
                 #define delaytime800ms   400*(2-factor)
                 #define delaytime1000ms 500*(2-factor)
				
			if(!stopcmd)
			{
				for(i=0;i<=4;i++)
				{
				   if (stopcmd)
     	                        {
     	                              break;
     	                        }
				   MOTOR_RIGHT_P(fast_temp,slow_temp);
				   msleep(1000);
				   MOTOR_STOP_P();
				   msleep(500);
				   if (stopcmd)
     	                        {
     	                              return;
     	                        }
				   MOTOR_LEFT_P(fast_temp,slow_temp);
				   msleep(1000);
				   MOTOR_STOP_P();
				   msleep(500);
				}
				   if (stopcmd)
				   {
					   return;
				   }
				   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_V_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_V_STOP_P();
				   msleep(200);
				if (stopcmd)
				{
					return;
				}
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
				if (stopcmd)
				{
					return;
				}

					MOTOR_LEFT_P(fast_temp,slow_temp);
					msleep(1500);
					MOTOR_STOP_P();
					msleep(500);
					if (stopcmd)
					{
						return;
					}
					MOTOR_RIGHT_P(fast_temp,slow_temp);
					msleep(1500);
					MOTOR_STOP_P();
					msleep(500);
				      if (stopcmd)
				     {
					  return;
				     }
		
				for(i=0;i<=1;i++)
				{
				  if (stopcmd)
     	                        {
     	                              break;
     	                        }
					MOTOR_FORWARD_P(fast_temp,slow_temp);
					msleep(1000);
					MOTOR_STOP_P();
					msleep(500);
				}
				if (stopcmd)
					{
					return;
					}

				for(i=0;i<=1;i++)
				{
						if (stopcmd)
     	                                  {
     	                                        break;
     	                                   }
					MOTOR_BACK_P(fast_temp,slow_temp);
					msleep(1000);
					MOTOR_STOP_P();
					msleep(500);
				}
				if (stopcmd)
				{
					return;
				}

				for(i=0;i<=1;i++)
				{
				if (stopcmd)
     	                        {
     	                              break;
     	                        }
					MOTOR_RIGHT_P(fast_temp,slow_temp);
					msleep(1500);
					MOTOR_STOP_P();
					msleep(500);
				}
				if (stopcmd)
					{
					return;
					}
				for(i=0;i<=1;i++)
				{
				if (stopcmd)
     	                        {
     	                              break;
     	                        }
					MOTOR_LEFT_P(fast_temp,slow_temp);
					msleep(1500);
					MOTOR_STOP_P();
					msleep(500);
				}
				if (stopcmd)
				{
					return;
				}		  
					MOTOR_FORWARD_P(fast_temp,slow_temp);
					msleep(1500);
					MOTOR_STOP_P();
					msleep(500);
				if (stopcmd)
				{
					return;
				}
					MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_V_STOP_P();
					msleep(200);
					MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_V_STOP_P();
					msleep(200);
				if (stopcmd)
				{
					return;
				}
					MOTOR_BACK_P(fast_temp,slow_temp);
					msleep(1500);
					MOTOR_STOP_P();
					msleep(500);
				if (stopcmd)
				{
					return;
				}
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
				if (stopcmd)
				{
					return;
				}
					 MOTOR_LEFT_P(fast_temp,slow_temp);
					 msleep(3000);
					 MOTOR_STOP_P();
					 msleep(500);
				if (stopcmd)
				{
					return;
				}
				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
				msleep(200);
				MOTOR_H_STOP_P();
				msleep(200);
				if (stopcmd)
					{
					return;
					}
					 MOTOR_RIGHT_P(fast_temp,slow_temp);
					 msleep(3000);
					 MOTOR_STOP_P();
					 msleep(500);
				if (stopcmd)
				{
					return;
				}
					 MOTOR_FORWARD_P(fast_temp,slow_temp);
					 msleep(800);
					 MOTOR_STOP_P();
					 msleep(500);
				if (stopcmd)
     	                        {
     	                              return;
     	                        }
					 MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
					 msleep(200);
					 MOTOR_V_STOP_P();
					 msleep(200);
					 MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
					 msleep(100);
					 MOTOR_V_STOP_P();
					 msleep(200);
				if (stopcmd)
				{
				 return;
				}
					 MOTOR_BACK_P(fast_temp,slow_temp);
					 msleep(800);
					 MOTOR_STOP_P();
					 msleep(500);
				 if (stopcmd)
     	                        {
     	                              return;
     	                        }

			}	
					MOTOR_H_STOP_P();
					MOTOR_V_STOP_P();
					MOTOR_STOP_P();

}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_35_P(unsigned int fast,unsigned int slow)
{
				unsigned int i_idex;
				unsigned int i;
				unsigned int factor;//速度因子
				unsigned fast_temp=0;
				unsigned slow_temp=0;
				fast_temp=fast;
				slow_temp=slow;
				
				if ((1==fast_temp)&&(0==slow_temp))
				{
				factor=0;
				}
				
				if ((1==slow_temp)&&(0==fast_temp))
				{
				factor=1;
				}
				
                 #define delaytime400ms   200*(2-factor)
                 #define delaytime600ms   300*(2-factor)
                 #define delaytime800ms   400*(2-factor)
                 #define delaytime1000ms 500*(2-factor)

					
		if(!stopcmd)
			{
			   for(i=0;i<=1;i++)
			   {
				 if (stopcmd)
     	                        {
     	                              break;
     	                        }
					   MOTOR_RIGHT_P(fast_temp,slow_temp);
					   msleep(1000);
					   MOTOR_STOP_P();
					   msleep(500);
				 if (stopcmd)
     	                        {
     	                              return;
     	                        }
					   MOTOR_LEFT_P(fast_temp,slow_temp);
					   msleep(1000);
					   MOTOR_STOP_P();
					   msleep(500);
			   }
					   if (stopcmd)
					   {
						   return;
					   }
					   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
					   msleep(200);
					   MOTOR_V_STOP_P();
					   msleep(200);
					   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
					   msleep(200);
					   MOTOR_V_STOP_P();
					   msleep(200);
					if (stopcmd)
					{
						return;
					}
						MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_H_STOP_P();
						msleep(200);
						MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_H_STOP_P();
						msleep(200);
						MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_H_STOP_P();
						msleep(200);
						MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_H_STOP_P();
						msleep(200);
					if (stopcmd)
					{
						return;
					}

					MOTOR_LEFT_ANTICLOCK();
					msleep(3000);
					MOTOR_STOP_P();
					msleep(500);
				if (stopcmd)
     	                        {
     	                              return;
     	                        }
					MOTOR_RIGHT_ANTICLOCK();
					msleep(3000);
					MOTOR_STOP_P();
					msleep(500);
					if (stopcmd)
					{
						return;
					}
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					if (stopcmd)
						{
						return;
						}
					for(i=0;i<=2;i++)
					{
					    if (stopcmd)
     	                                   {
     	                                        break;
     	                                    }
						MOTOR_FORWARD_P(fast_temp,slow_temp);
						msleep(500);
						MOTOR_STOP_P();
						msleep(500);
					}
					if (stopcmd)
					{
						return;
					}
						MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						for(i=0;i<=2;i++)
						{
							if (stopcmd)
     	                                           {
                       	                              break;
                      	                      }
						MOTOR_BACK_P(fast_temp,slow_temp);
						msleep(500);
						MOTOR_STOP_P();
						msleep(500);
						}
					if (stopcmd)
					{
						return;
					}
						MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
					if (stopcmd)
						{
						return;
						}
					for(i=0;i<=2;i++)
					{
						if (stopcmd)
     	                                    {
                	                              break;
               	                      }
						MOTOR_RIGHT_P(fast_temp,slow_temp);
						msleep(2000);
						MOTOR_STOP_P();
						msleep(500);
					}
					if (stopcmd)
					{
						return;
					}
					for(i=0;i<=2;i++)
					{
				          if (stopcmd)
     	                                {
     	                                        break;
     	                                }
						MOTOR_LEFT_P(fast_temp,slow_temp);
						msleep(2000);
						MOTOR_STOP_P();
						msleep(500);
					}	
					if (stopcmd)
					{
						return;
					}
						MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
					if (stopcmd)
					{
						return;
					}			  
						MOTOR_FORWARD_P(fast_temp,slow_temp);
						msleep(1500);
						MOTOR_STOP_P();
						msleep(500);
					if (stopcmd)
					{
						return;
					}
						MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
						MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_V_STOP_P();
						msleep(200);
					if (stopcmd)
					{
						return;
					}
						MOTOR_BACK_P(fast_temp,slow_temp);
						msleep(1500);
						MOTOR_STOP_P();
						msleep(500);
					if (stopcmd)
					{
						return;
					}
						MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_H_STOP_P();
						msleep(200);
						MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_H_STOP_P();
						msleep(200);
						MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_H_STOP_P();
						msleep(200);
						MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
						msleep(200);
						MOTOR_H_STOP_P();
						msleep(200);
					if (stopcmd)
					{
						return;
					}  
						 MOTOR_LEFT_P(fast_temp,slow_temp);
						 msleep(3000);
						 MOTOR_STOP_P();
						 msleep(500);
					if (stopcmd)
					{
						return;
					}

						 MOTOR_RIGHT_P(fast_temp,slow_temp);
						 msleep(3000);
						 MOTOR_STOP_P();
						 msleep(500);
					if (stopcmd)
					{
						return;
					}
						 MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
						 msleep(200);
						 MOTOR_V_STOP_P();
						 msleep(200);
						 MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
						 msleep(200);
						 MOTOR_V_STOP_P();
						 msleep(200);
					if (stopcmd)
					{
					return;
					} 

					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					if (stopcmd)
					{
						return;
					}
				}	
						MOTOR_H_STOP_P();
						MOTOR_V_STOP_P();
						MOTOR_STOP_P();

}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_42_P(unsigned int fast,unsigned int slow)
{
	
	unsigned int i_idex;
	unsigned int factor;//速度因子
	unsigned fast_temp=0;
	unsigned slow_temp=0;
	fast_temp=fast;
	slow_temp=slow;
	
	if ((1==fast_temp)&&(0==slow_temp))
	{
	factor=0;
	}
	
	if ((1==slow_temp)&&(0==fast_temp))
	{
	factor=1;
	}
	
	
#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)
	
	if(!stopcmd)
	{
		    MOTOR_BACK_P(fast_temp,slow_temp);		
		    msleep(1000); 
		    MOTOR_STOP_P(); 
		    msleep(500);
		 if (stopcmd)
     	            {
     	                     return;
     	             }
		    MOTOR_FORWARD_P(fast_temp,slow_temp);
		    msleep(1000);
		    MOTOR_STOP_P();
		    msleep(500);
		   if (stopcmd)
     	           {
     	                     return;
     	            }
		    MOTOR_FORWARD_P(fast_temp,slow_temp);
		    msleep(1000);
		    MOTOR_STOP_P();
		    msleep(500);
		    if (stopcmd)
     	             {
     	                    return;
     	             }
		    MOTOR_BACK_P(fast_temp,slow_temp);		
		    msleep(1000); 
		    MOTOR_STOP_P(); 
		    msleep(500);
			   for (i_idex=0;i_idex<=5;i_idex++)
			  {
				 if (stopcmd)
				{
					break;
				}
			MOTOR_RIGHT_P(fast_temp,slow_temp);
			msleep(500);
			MOTOR_STOP_P();
			msleep(500);
			 }
			if (stopcmd)
			{
				  return;
			}	 
			for (i_idex=0;i_idex<=5;i_idex++)
			  {
				 if (stopcmd)
				{
					break;
				}
			MOTOR_LEFT_P(fast_temp,slow_temp);
			msleep(500);
			MOTOR_STOP_P();
			msleep(500);
			   }
			if (stopcmd)
			{
				  return;
			}	 

		  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_V_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_V_STOP_P();
		  msleep(200);
		  if (stopcmd)
     	          {
     	                 return;
     	           }
			
		  MOTOR_RIGHT_P(fast_temp,slow_temp);
		  msleep(2000);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	           {
     	                     return;
     	            }
		  MOTOR_LEFT_P(fast_temp,slow_temp);
		  msleep(2000);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	           {
     	                    return;
     	            }
		  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
		  msleep(200);
		  MOTOR_H_STOP_P();
		  msleep(200);
		  if (stopcmd)
			  {
			  return;
			  }
		  MOTOR_LEFT_ANTICLOCK();
		  msleep(2500);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	             {
     	                    return;
     	              }
		  MOTOR_LEFT_CLOCKDIR();
		  msleep(2500);
		  MOTOR_STOP_P();
		  msleep(500);
		  if (stopcmd)
     	             {
     	                    return;
     	              }

		  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_V_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_V_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_V_STOP_P();
		  msleep(200);
		  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		  msleep(200);
		  MOTOR_V_STOP_P();
		  msleep(200);
		if (stopcmd)
		{
			return;
		}

			MOTOR_FORWARD_P(fast_temp,slow_temp);
			msleep(1500);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
     	          {
     	                return;
     	           }
			MOTOR_BACK_P(fast_temp,slow_temp);
			msleep(1500);
			MOTOR_STOP_P(); 
			msleep(500);
		if (stopcmd)
		{
			return;
		}	  
		   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		   if (stopcmd)
		  {
			   return;
		  }

		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		if (stopcmd)
		{
		   return;
		}
	  
		   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
		   msleep(200);
		   MOTOR_V_STOP_P();
		   msleep(200);
		   if (stopcmd)
		  {
			   return;
		  }

		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		if (stopcmd)
		{
		   return;
		}
		
		}
		   MOTOR_H_STOP_P();
		   MOTOR_V_STOP_P();
		   MOTOR_STOP_P();
}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_43_P(unsigned int fast,unsigned int slow)
{		
		unsigned int i_idex;
		unsigned int factor;//速度因子
		unsigned fast_temp=0;
		unsigned slow_temp=0;
		fast_temp=fast;
		slow_temp=slow;
		
		if ((1==fast_temp)&&(0==slow_temp))
		{
		factor=0;
		}
		
		if ((1==slow_temp)&&(0==fast_temp))
		{
		factor=1;
		}
		
		
#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)
		
	if(!stopcmd)
	{
			MOTOR_FORWARD_P(fast_temp,slow_temp);
			msleep(1000);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
     	        {
     	                    return;
     	         }
			MOTOR_BACK_P(fast_temp,slow_temp);		
			msleep(1000); 
			MOTOR_STOP_P(); 
			msleep(500);
		if (stopcmd)
     	         {
     	                    return;
     	          }
			MOTOR_BACK_P(fast_temp,slow_temp);		
			msleep(1000); 
			MOTOR_STOP_P(); 
			msleep(500);
		if (stopcmd)
     	          {
     	                    return;
     	           }
			MOTOR_FORWARD_P(fast_temp,slow_temp);
			msleep(1000);
			MOTOR_STOP_P();
			msleep(500);
		if (stopcmd)
     	         {
     	                    return;
     	          }
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		if (stopcmd)
			{
			return;
			}
			  
		   for (i_idex=0;i_idex<=2;i_idex++)
			{
					 if (stopcmd)
					{
						break;
					}
				MOTOR_LEFT_P(fast_temp,slow_temp);
				msleep(2000);
				MOTOR_STOP_P();
				msleep(500);
				  
			}
	                        if (stopcmd)
	                      	{
	                             	return;
                       		}
			for (i_idex=0;i_idex<=2;i_idex++)
				{
					 if (stopcmd)
					{
						break;
					}
				MOTOR_RIGHT_P(fast_temp,slow_temp);
				msleep(2000);
				MOTOR_STOP_P();
				msleep(500);
				   }
				if (stopcmd)
				{
					  return;
				}

			  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
		if (stopcmd)
     	             {
     	                    return;
     	              }
			  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			if (stopcmd)
			{
				return;
			}			

			  MOTOR_RIGHT_ANTICLOCK();
			  msleep(3000);
			  MOTOR_STOP_P();
			  msleep(500);
		if (stopcmd)
     	           {
     	                    return;
     	           }
			  MOTOR_RIGHT_CLOCKDIR();
			  msleep(3000);
			  MOTOR_STOP_P();
			  msleep(500);
		if (stopcmd)
     	          {
     	                    return;
     	           }
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
		msleep(200);
		MOTOR_H_STOP_P();
		msleep(200);
		if (stopcmd)
			{
			return;
			}
			  MOTOR_LEFT_ANTICLOCK();
			  msleep(2500);
			  MOTOR_STOP_P();
			  msleep(500);
		if (stopcmd)
     	             {
     	                    return;
     	              }
			  MOTOR_LEFT_CLOCKDIR();
			  msleep(2500);
			  MOTOR_STOP_P();
			  msleep(500);
			if (stopcmd)
		        {
				return;
		         }
			  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			if (stopcmd)
			{
				return;
			}
			  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			  msleep(200);
			  MOTOR_V_STOP_P();
			  msleep(200);
			if (stopcmd)
			{
				return;
			}
  
				MOTOR_FORWARD_P(fast_temp,slow_temp);
				msleep(1000);
				MOTOR_STOP_P();
				msleep(500);
		if (stopcmd)
     	             {
     	                    return;
     	              }
				MOTOR_BACK_P(fast_temp,slow_temp);
				msleep(1000);
				MOTOR_STOP_P(); 
				msleep(500);
			if (stopcmd)
			{
				return;
			}	  
			   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_V_STOP_P();
			   msleep(200);
			   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
			   msleep(200);
			   MOTOR_V_STOP_P();
			   msleep(200);
			   if (stopcmd)
			  {
				   return;
			  }
			 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			 msleep(200);
			 MOTOR_H_STOP_P();
			 msleep(200);
			 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			 msleep(200);
			 MOTOR_H_STOP_P();
			 msleep(200);
			 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
			 msleep(200);
			 MOTOR_H_STOP_P();
			 msleep(200);
			 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
			 msleep(200);
			 MOTOR_H_STOP_P();
			 msleep(200);
			if (stopcmd)
			{
			   return;
			}
		}
			   MOTOR_H_STOP_P();
			   MOTOR_V_STOP_P();
			   MOTOR_STOP_P();
}
/************************************************************/
/************************************************************/
/************************************************************/

void RobotDanceAction_44_P(unsigned int fast,unsigned int slow)
{
			unsigned int i_idex;
			unsigned int factor;//速度因子
			unsigned fast_temp=0;
			unsigned slow_temp=0;
			fast_temp=fast;
			slow_temp=slow;
			
			if ((1==fast_temp)&&(0==slow_temp))
			{
			factor=0;
			}
			
			if ((1==slow_temp)&&(0==fast_temp))
			{
			factor=1;
			}
			
#define delaytime400ms   200*(2-factor)
#define delaytime600ms   300*(2-factor)
#define delaytime800ms   400*(2-factor)
#define delaytime1000ms 500*(2-factor)
			
	if(!stopcmd)
	{
				MOTOR_FORWARD_P(fast_temp,slow_temp);
				msleep(1000);
				MOTOR_STOP_P();
				msleep(500);
		if (stopcmd)
     	             {
     	                    return;
     	              }
				MOTOR_BACK_P(fast_temp,slow_temp);		
				msleep(1000); 
				MOTOR_STOP_P(); 
				msleep(500);
		if (stopcmd)
     	             {
     	                    return;
     	              }	  
				 for (i_idex=0;i_idex<=3;i_idex++)
				 {
					if (stopcmd)
					{
							break;
				        }
					MOTOR_LEFT_P(fast_temp,slow_temp);
					msleep(1000);
					MOTOR_STOP_P();
					msleep(500);
				  }
					if (stopcmd)
						 {
						  return;
						 }	 
					for (i_idex=0;i_idex<=3;i_idex++)
					  {
						 if (stopcmd)
						{
							break;
						}
					MOTOR_RIGHT_P(fast_temp,slow_temp);
					msleep(1000);
					MOTOR_STOP_P();
					msleep(500);
					   }
					if (stopcmd)
						 {
						  return;
						 }
				for(i_idex=0;i_idex<=2;i_idex++)
				{
			            if (stopcmd)
     	                               {
     	                                   break;
     	                                }
				  MOTOR_FORWARD_P(fast_temp,slow_temp);
				  msleep(500);
				  MOTOR_STOP_P();
				  msleep(500);
				  if (stopcmd)
     	                          {
     	                               return;
     	                           }
				  MOTOR_BACK_P(fast_temp,slow_temp);
				  msleep(500);
				  MOTOR_STOP_P();
				  msleep(500);
				}
				if (stopcmd)
				{
					return;
				}
				  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				  if (stopcmd)
     	                        {
     	                               return;
     	                         }
				  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				if (stopcmd)
				{
					return;
				}			
				  MOTOR_RIGHT_P(fast_temp,slow_temp);
				  msleep(3000);
				  MOTOR_STOP_P();
				  msleep(500);
				  if (stopcmd)
				   {
					  return;
			           } 

				  MOTOR_LEFT_P(fast_temp,slow_temp);
				  msleep(3000);
				  MOTOR_STOP_P();
				  msleep(500);
				  if (stopcmd)
				  {
					  return;
				   } 
				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  if (stopcmd)
					  {
					  return;
					  }
				  MOTOR_RIGHT_ANTICLOCK();
				  msleep(2000);
				  MOTOR_STOP_P();
				  msleep(500);
				  if (stopcmd)
				   {
					  return;
			           }   
  
				  MOTOR_LEFT_ANTICLOCK();
				  msleep(2000);
				  MOTOR_STOP_P();
				  msleep(500);
				  if (stopcmd)
				{
					  return;
				}   
				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
				  msleep(200);
				  MOTOR_H_STOP_P();
				  msleep(200);
				  if (stopcmd)
					  {
					  return;
					  }
				  MOTOR_RIGHT_CLOCKDIR();
				  msleep(2000);
				  MOTOR_STOP_P();
				  msleep(500);
				  if (stopcmd)
				{
					  return;
				} 
				  MOTOR_LEFT_CLOCKDIR();
				  msleep(2000);
				  MOTOR_STOP_P();
				  msleep(500);
				if (stopcmd)
				{
					return;
				}
				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				  msleep(200);
				  MOTOR_V_STOP_P();
				  msleep(200);
				if (stopcmd)
				{
					return;
				}
	  
				   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_V_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_V_STOP_P();
				   msleep(200);
				   if (stopcmd)
				  {
					   return;
				  }
				   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_V_STOP_P();
				   msleep(200);
				   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
				   msleep(200);
				   MOTOR_V_STOP_P();
				   msleep(200);
				if (stopcmd)
				{
					return;
				}
				 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				 msleep(200);
				 MOTOR_H_STOP_P();
				 msleep(200);
				if (stopcmd)
				{
				   return;
				}
				}
				   MOTOR_H_STOP_P();
				   MOTOR_V_STOP_P();
				   MOTOR_STOP_P();
}

/************************************************************/
/************************************************************/
/************************************************************/
void RobotDanceAction_45_P(unsigned int fast,unsigned int slow)
{
				unsigned int i_idex;
				unsigned int factor;//速度因子
				unsigned fast_temp=0;
				unsigned slow_temp=0;
				fast_temp=fast;
				slow_temp=slow;
				
				if ((1==fast_temp)&&(0==slow_temp))
				{
				factor=0;
				}
				
				if ((1==slow_temp)&&(0==fast_temp))
				{
				factor=1;
				}
				
                             #define delaytime400ms   200*(2-factor)
                             #define delaytime600ms   300*(2-factor)
                             #define delaytime800ms   400*(2-factor)
                             #define delaytime1000ms 500*(2-factor)
				
				if(!stopcmd)
				{
					MOTOR_FORWARD_P(fast_temp,slow_temp);
					msleep(1000);
					MOTOR_STOP_P();
					msleep(500);
					if (stopcmd)
					{
						return;
					}	
					MOTOR_BACK_P(fast_temp,slow_temp);		
					msleep(1000); 
					MOTOR_STOP_P(); 
					msleep(500);
					if (stopcmd)
					{
						return;
					}				  
					for (i_idex=0;i_idex<=2;i_idex++)
					{
					     if (stopcmd)
						{
							break;
						}
						MOTOR_LEFT_P(fast_temp,slow_temp);
						msleep(1500);
						MOTOR_STOP_P();
						msleep(500);
						if (stopcmd)
						{
							return;
						}	
						MOTOR_RIGHT_P(fast_temp,slow_temp);
						msleep(1500);
						MOTOR_STOP_P();
						msleep(500);
					}
					     if (stopcmd)
						 {
							  return;
						}	 
						 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
						 msleep(200);
						 MOTOR_H_STOP_P();
						 msleep(200);
						 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
						 msleep(200);
						 MOTOR_H_STOP_P();
						 msleep(200);
						 MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
						 msleep(200);
						 MOTOR_H_STOP_P();
						 msleep(200);
						 MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	 
						 msleep(200);
						 MOTOR_H_STOP_P();
						 msleep(200);
						 if (stopcmd)
							 {
							 return;
							 }

					for(i_idex=0;i_idex<=2;i_idex++)
					{
						if (stopcmd)
						{
							break;
						}	
					    MOTOR_FORWARD_P(fast_temp,slow_temp);
					    msleep(500);
					    MOTOR_STOP_P();
					    msleep(500);
						if (stopcmd)
						{
							return;
						}	
					    MOTOR_BACK_P(fast_temp,slow_temp);
					    msleep(500);
					    MOTOR_STOP_P();
					    msleep(500);
					}
					   if (stopcmd)
					    {
						return;
					    }
					  MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
					  msleep(200);
					  MOTOR_V_STOP_P();
					  msleep(200);
					  MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
					  msleep(200);
					  MOTOR_V_STOP_P();
					  msleep(200);
					  if (stopcmd)
					   {
						  return;
					    }   	

					  MOTOR_RIGHT_ANTICLOCK();
					  msleep(1500);
					  MOTOR_STOP_P();
					  msleep(500);
					  if (stopcmd)
					   {
						  return;
					    }   
					  MOTOR_RIGHT_CLOCKDIR();
					  msleep(1500);
					  MOTOR_STOP_P();
					  msleep(500);
					if (stopcmd)
					{
						return;
					}
					  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					  msleep(200);
					  MOTOR_V_STOP_P();
					  msleep(200);
					  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					  msleep(200);
					  MOTOR_V_STOP_P();
					  msleep(200);
					  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					  msleep(200);
					  MOTOR_V_STOP_P();
					  msleep(200);
					  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					  msleep(200);
				         MOTOR_V_STOP_P();
					  msleep(200);
					if (stopcmd)
					{
						return;
					}
					  MOTOR_LEFT_P(fast_temp,slow_temp);
					  msleep(2500);
					  MOTOR_STOP_P();
					  msleep(500);
					  if (stopcmd)
					   {
						  return;
					    }   
					  MOTOR_RIGHT_P(fast_temp,slow_temp);
					  msleep(2500);
					  MOTOR_STOP_P();
					  msleep(500);
					  if (stopcmd)
					  {
						return;
					   }	  
					  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
					  msleep(200);
					  MOTOR_H_STOP_P();
					  msleep(200);
					  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
					  msleep(200);
					  MOTOR_H_STOP_P();
					  msleep(200);
					  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
					  msleep(200);
					  MOTOR_H_STOP_P();
					  msleep(200);
					  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	  
					  msleep(200);
					  MOTOR_H_STOP_P();
					  msleep(200);
					  if (stopcmd)
						  {
						  return;
						  }
						MOTOR_FORWARD_P(fast_temp,slow_temp);
						msleep(1000);
						MOTOR_STOP_P();
						msleep(500);
						if (stopcmd)
						{
							return;
						}	
						MOTOR_BACK_P(fast_temp,slow_temp);
						msleep(1000);
						MOTOR_STOP_P(); 
					if (stopcmd)
					{
						return;
					}	  
					   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
					   msleep(200);
					   MOTOR_V_STOP_P();
					   msleep(200);
					   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
					   msleep(200);
					   MOTOR_V_STOP_P();
					   msleep(200);
					   if (stopcmd)
					  {
						   return;
					  }
					   MOTOR_HEAD_FORWARD_P(fast_temp,slow_temp);
					   msleep(200);
					   MOTOR_V_STOP_P();
					   msleep(200);
					   MOTOR_HEAD_BACK_P(fast_temp,slow_temp);
					   msleep(200);
					   MOTOR_V_STOP_P();
					   msleep(200);
					if (stopcmd)
					{
						return;
					}
					MOTOR_RIGHT_ANTICLOCK();
					msleep(2000);
					MOTOR_STOP_P();
					msleep(500);
					if (stopcmd)
					{
						return;
					}	
					MOTOR_RIGHT_CLOCKDIR();
					msleep(2000);
					MOTOR_STOP_P();
					msleep(500);
					if (stopcmd)
					{
						return;
					}	
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp); 
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);	
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					if (stopcmd)
						{
						return;
						}
					MOTOR_LEFT_ANTICLOCK();
					msleep(2000);
					MOTOR_STOP_P();
					msleep(500);
					if (stopcmd)
					{
						return;
					}	
					MOTOR_LEFT_CLOCKDIR();
					msleep(2000);
					MOTOR_STOP_P();
					msleep(500);
					if (stopcmd)
					{
						return;
					}					
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					msleep(200);
					MOTOR_H_STOP_P();
					msleep(200);
					if (stopcmd)
					{
					   return;
					}
				}
					   MOTOR_H_STOP_P();
					   MOTOR_V_STOP_P();
					   MOTOR_STOP_P();
}
/************************************************************/
/************************************************************/
/************************************************************/
