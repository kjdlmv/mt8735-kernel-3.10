#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
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
#include <linux/delay.h>

#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>

#include "motor_drv.h"
#include "robot_dance.h"

#include <mach/battery_common.h>

//#define TEST_DANCE_H
#define MOTOR_PWM_H
//#define MOTOR_NON_PWM_H

#define  BIT0 0x00000001
#define  BIT1 0x00000002
#define  BIT2 0x00000004
#define  BIT3 0x00000008
#define MASK 0x00000000

#define DEV_NAME   "motor"
static struct cdev *motor_cdev;
static dev_t m_dev;
static struct class *motor_class = NULL;
struct device *motor_device = NULL;
static unsigned int h_idle=0;
static unsigned int rand_num=1;
char debuf[10];

unsigned int factor=1;  //机器人速度快慢选择0-快,1-慢
unsigned int fast_temp=1;
unsigned int slow_temp=0;
unsigned int stop_flag=0;
unsigned int h_stop_flag=0;

//extern PMU_ChargerStruct BMT_status;

#define  delaytime400ms  200*(2-factor)
#define  delaytime600ms  300*(2-factor)
#define  delaytime800ms  400*(2-factor)
#define  delaytime1000ms 500*(2-factor)

 unsigned int robotdance_start=0;  //机器人开始跳舞标志位
static struct task_struct *thread = NULL;//开机器人跳舞线程

//static unsigned int vol_nomal=0;
static unsigned int ret_limit=0;

int limitswitch;  //头部限位信号
unsigned int stopcmd=0;//机器人停止跳舞标志位
void motor_h_stop(int val);
extern void aw2013_breath_all(int led0,int led1,int led2) ;

/*
#define MOTOR_INIT()   \
{\
    mt_set_gpio_mode(GPIO44_PIN, GPIO_MODE_00);  \ 
    mt_set_gpio_dir(GPIO44_PIN, GPIO_DIR_OUT) ;   \
    mt_set_gpio_mode(GPIO10_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO10_PIN, GPIO_DIR_OUT) ;  \    
    mt_set_gpio_mode(GPIO95_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO95_PIN, GPIO_DIR_OUT) ;	\
    mt_set_gpio_mode(GPIO96_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO96_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO60_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO60_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO99_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO99_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO100_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO100_PIN, GPIO_DIR_OUT) ;  \
	mt_set_gpio_mode(GPIO101_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO101_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO102_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO102_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO42_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO42_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO43_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO43_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO44_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO44_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO10_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO10_PIN, GPIO_DIR_OUT) ;\    
    mt_set_gpio_mode(GPIO43_PIN,GPIO_MODE_00); \
    mt_set_gpio_dir(GPIO43_PIN, GPIO_DIR_OUT);\
}
*/
#define MOTOR_INIT()   \
{\
    mt_set_gpio_mode(GPIO44_PIN, GPIO_MODE_00);  \ 
    mt_set_gpio_dir(GPIO44_PIN, GPIO_DIR_OUT) ;   \
    mt_set_gpio_mode(GPIO10_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO10_PIN, GPIO_DIR_OUT) ;  \    
    mt_set_gpio_mode(GPIO60_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO60_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO21_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO21_PIN, GPIO_DIR_OUT) ;  \   
	mt_set_gpio_mode(GPIO101_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO101_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO102_PIN, GPIO_MODE_00);  \
    mt_set_gpio_dir(GPIO102_PIN, GPIO_DIR_OUT) ;  \
    mt_set_gpio_mode(GPIO43_PIN,GPIO_MODE_00); \
    mt_set_gpio_dir(GPIO43_PIN, GPIO_DIR_OUT);\
}

#define POWER_EN()   \			
{\		
	mt_set_gpio_out(GPIO101_PIN, 1);  \
	mt_set_gpio_out(GPIO102_PIN, 1);\ 
	mt_set_gpio_out(GPIO43_PIN,1);\
}
#define POWER_CLOSE()   \			
{\
	mt_set_gpio_out(GPIO101_PIN, 0);  \
	mt_set_gpio_out(GPIO102_PIN, 0);\ 
	mt_set_gpio_out(GPIO43_PIN,0);\
}
#define MOTOR_GPIO_POWER()     mt_set_gpio_out(GPIO43_PIN, 1)  
#define MOTOR_GPIO_POWER_CLOSE()     mt_set_gpio_out(GPIO43_PIN, 0)  


#define POWER_V_CLOSE()   	mt_set_gpio_out(GPIO100_PIN, 0) 	
#define POWER_H_CLOSE()   	mt_set_gpio_out(GPIO99_PIN, 0)
#define POWER_V_OPEN()   	mt_set_gpio_out(GPIO100_PIN, 1) 	
#define POWER_H_OPEN()   	mt_set_gpio_out(GPIO99_PIN, 1)

/***********************************************************************/
/*******************************************************************/
#ifdef MOTOR_PWM_H
/*******************************************************************/
#define MOTOR_FORWARD()   \			
 {\
	 mt_set_gpio_out(GPIO44_PIN, 0); \
	 mt_set_gpio_out(GPIO10_PIN, 1); \
	 mt_set_gpio_out(GPIO60_PIN, 1); \
	 mt_set_gpio_out(GPIO21_PIN, 0);\
 }

#define MOTOR_BACK()   \			
 {\
	 mt_set_gpio_out(GPIO44_PIN, 1); \
	 mt_set_gpio_out(GPIO10_PIN, 0); \
	 mt_set_gpio_out(GPIO60_PIN, 0); \
	 mt_set_gpio_out(GPIO21_PIN, 1);\
 }

#define MOTOR_RIGHT()   \			
 {\
	 mt_set_gpio_out(GPIO44_PIN, 0); \
	 mt_set_gpio_out(GPIO10_PIN, 1); \
	 mt_set_gpio_out(GPIO60_PIN, 0); \
	 mt_set_gpio_out(GPIO21_PIN, 1);\
 }
 
#define MOTOR_LEFT()   \			
 {\
	 mt_set_gpio_out(GPIO44_PIN, 1); \
	 mt_set_gpio_out(GPIO10_PIN, 0); \
	 mt_set_gpio_out(GPIO60_PIN, 1); \
	 mt_set_gpio_out(GPIO21_PIN, 0);\
 }
 
#define MOTOR_STOP()   \			
 {\
	 mt_set_gpio_out(GPIO44_PIN, 0); \
	 mt_set_gpio_out(GPIO10_PIN, 0); \
	 mt_set_gpio_out(GPIO60_PIN, 0); \
	 mt_set_gpio_out(GPIO21_PIN, 0);\
 }
 

#define MOTOR_HEAD_BACK()   \			
 {\  
	 mt_set_gpio_out(GPIO95_PIN, 1); \
	 mt_set_gpio_out(GPIO96_PIN, 0);\
 }
#define MOTOR_HEAD_FORWARD()   \			
 {\  
	 mt_set_gpio_out(GPIO95_PIN, 0); \
	 mt_set_gpio_out(GPIO96_PIN, 1);\
 }
 
#define MOTOR_V_STOP()   \			
 {\  
	 mt_set_gpio_out(GPIO95_PIN, 0); \
	 mt_set_gpio_out(GPIO96_PIN, 0);\
 }
 
#define MOTOR_HEAD_RIGHT()   \			
 {\
	 mt_set_gpio_out(GPIO42_PIN, 1); \
	 mt_set_gpio_out(GPIO44_PIN, 0);\	 
 }
 
#define MOTOR_HEAD_LEFT()   \			
 {\
	 mt_set_gpio_out(GPIO42_PIN, 0); \
	 mt_set_gpio_out(GPIO44_PIN, 1);\	 
 }
 
#define MOTOR_H_STOP()   \			
 {\  
	 mt_set_gpio_out(GPIO42_PIN, 0); \
	 mt_set_gpio_out(GPIO44_PIN, 0);\
 }
 
  void motor_h_stop(int val)
 {	   
  
	 limitswitch=val;
	 MOTOR_H_STOP_P();
	 h_idle=0;
	 ret_limit=val;
 }


 static void robot_forwad(uint32_t arg)
 {
	 MOTOR_FORWARD();
 }
 static void robot_back(uint32_t arg)
 {
	 MOTOR_BACK();
 
 }
 static void robot_stop(uint32_t arg)
 {
	 MOTOR_STOP();
 
 }
 
 static void motor_drive(unsigned int cmd,uint32_t arg)
 {
		 switch(cmd)
		 {
			 case  ROBOT_FORWORD:
				 robot_forwad(arg);
				 break;
			 case  ROBOT_BACK:
				 robot_back(arg);
				 break;
			 case ROBOT_LEFT:
				 //robot_left(arg);
				 break;
			 case ROBOT_RIGHT:
				 //robot_right(arg);
				 break; 
			 case ROBOT_STOP:
				 robot_stop(arg);
				 break; 
		 }
		 
 }
 static void motor_head_drive(unsigned int cmd,uint32_t arg)
 {
		 switch(cmd)
		 {
			 case  ROBOT_HEAD_UP:
				 //robot_head_up(arg);
				 break;
			 case  ROBOT_HEAD_DOWN:
				 //robot_head_down(arg);
				 break;
			 case  ROBOT_HEAD_LEFT:
				 //robot_head_left(arg);
				 break;
			 case  ROBOT_HEAD_RIGHT:
				 //robot_head_right(arg);
				 break; 
			 case  ROBOT_HEAD_STOP:
				 //robot_head_stop(arg);
				 break; 
			 
		 }
		 
 }


 static long motor_unlocked_ioctl (struct file *pfile, unsigned int cmd, unsigned long param)
 {
	         long err = 0;
		 void __user *ptr = (void __user*) param;
		 int dat;
		 uint32_t arg;
		 switch (cmd)
		 {
			 case  ROBOT_FORWORD:
			 case  ROBOT_BACK:
			 case  ROBOT_LEFT:
			 case  ROBOT_RIGHT:
				 if(copy_from_user(&arg, ptr, sizeof(arg)))
				 {
					 err = -EFAULT;
					 goto err_out;
				 }
//				 motor_drive(cmd,arg);
				 break;
			 
			 case ROBOT_HEAD_UP:
			 case ROBOT_HEAD_DOWN:
			 case ROBOT_HEAD_LEFT:
			 case ROBOT_HEAD_RIGHT:
				 if(copy_from_user(&arg, ptr, sizeof(arg)))
				 {
					 err = -EFAULT;
					 goto err_out;
				 }
//				 motor_head_drive(cmd,arg);
				 break;
				 
		 }
		 
		 err_out:
		 return err;	
 }
 
 
 static int motor_release (struct inode *node, struct file *file)
 {

	 return 0;
 
 }
 static int motor_open (struct inode *inode, struct file *file)
 {
	 MOTOR_INIT();
	 POWER_EN();
	// printk("motor  alloc_77777777777");
	 return nonseekable_open(inode, file);
 }

static unsigned char user_cmd,user_param;

static void robot_stop_dance(void)
{
	
	MOTOR_H_STOP_P();
	MOTOR_V_STOP_P();
	MOTOR_STOP_P();

	POWER_CLOSE();
	POWER_H_CLOSE();
	POWER_V_CLOSE();
}
static ssize_t check_state_show (struct device *dev, struct device_attribute *attr,char *buf)
{
	int len=0;
	
	len += sprintf(buf + len, "%c,", user_cmd);
	len += sprintf(buf + len, "%d,", user_param);
	len += sprintf(buf + len, "%d,", limitswitch);
	return len;
}



static int kkthread(void *unused) 
{
	static unsigned int v_idle=0,up=1,down=1;
	static unsigned int idle=0,dance_flag=0; 
	printk("daviekuo start kthread_run %c  %d  %d\n", user_cmd, idle, robotdance_start);

	while(!kthread_should_stop())
	{
	  	  msleep(10);		
              if (9==ret_limit ||10==ret_limit)
              	{
              	   MOTOR_H_STOP_P();
		   POWER_H_CLOSE();
    printk("daviekuo  to  run kkthread %c  %d  %d\n", user_cmd, idle, robotdance_start);          	}
			  
		switch(user_cmd)
		 {
		 
			case C_ROBOT_FORWORD:

				if(0 == idle && robotdance_start==0)
				{
				POWER_EN();
 				MOTOR_FORWARD_P(fast_temp,slow_temp);
				idle =1;
				}
			       break;
			case C_ROBOT_BACK:
				if(0 ==idle && robotdance_start==0)
				{
				POWER_EN();
 				MOTOR_BACK_P(fast_temp,slow_temp);
				idle=1;
				}
			        break;
			case  C_ROBOT_LEFT:
				if(0 == idle && robotdance_start==0)
				{
				idle =1;
				POWER_EN();
 				MOTOR_LEFT_P(fast_temp,slow_temp);
				}
			       break;
			case  C_ROBOT_RIGHT:
				if(0 == idle&& robotdance_start==0)
				{
				idle =1;
				POWER_EN();
 				MOTOR_RIGHT_P(fast_temp,slow_temp);
				}
			       break;
			case C_ROBOT_STOP:
				if(1 == idle && robotdance_start==0)
				{
				stop_flag=1;
				MOTOR_STOP_P();
				POWER_CLOSE();
				idle=0;
				}				
				break;	
			case C_ROBOT_HEAD_UP:
				if((v_idle == 0)&& (robotdance_start==0)&&(up==1))
				{
				v_idle=1;
				h_stop_flag=0;
				POWER_V_OPEN();
 				MOTOR_HEAD_FORWARD_P(1,0); 
				msleep(100);
 				MOTOR_V_STOP_P();
				POWER_V_CLOSE();
				up=0;
				down=1;
				}
				break;
			case C_ROBOT_HEAD_DOWN:
				if((v_idle ==0)&&(robotdance_start==0)&&(down==1))
				{
				v_idle=1;
				h_stop_flag=0;
				POWER_V_OPEN();
 				MOTOR_HEAD_BACK_P(1,0);
				msleep(100);
 				MOTOR_V_STOP_P();
				POWER_V_CLOSE();
				up=1;
				down=0;
				}
				break;
			case C_ROBOT_V_STOP:

				break;
			case C_ROBOT_HEAD_RIGHT:
				if(0 ==h_idle && robotdance_start==0 &&  ret_limit !=9)
				{
				h_idle=1;
				POWER_H_OPEN();
 				MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				if(ret_limit ==10)ret_limit=0; ///left  limit;				
				}
				break;
			case C_ROBOT_HEAD_LEFT:
				if(0 ==h_idle && robotdance_start==0 &&  ret_limit !=10)
				{
				h_idle=1;
				POWER_H_OPEN();
 				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				if(ret_limit ==9)ret_limit=0; ///right limit;
				}
				break;
			case C_ROBOT_H_STOP:
				if(1 ==h_idle && robotdance_start==0)
				{
				h_stop_flag=1;
				MOTOR_H_STOP_P();
				POWER_H_CLOSE();
				h_idle=0;
				}
				break;

			case C_ROBOT_STOP_DANCE:		
			    if(0==robotdance_start && dance_flag ==1)
			    	{
				
				idle =0;
				dance_flag=0;
 				MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
				msleep(delaytime1000ms*2);
				if (10==limitswitch)
				{
				  MOTOR_H_STOP_P();
				  msleep(300);
 				  MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
				  msleep(delaytime400ms*2);
 				}
				MOTOR_H_STOP_P();
				MOTOR_V_STOP_P();
				MOTOR_STOP_P();				
				//aw2013_breath_all(0,0,0);
				POWER_CLOSE();
				POWER_H_CLOSE();
				POWER_V_CLOSE();
			    	}
				break;
			case C_ROBOT_START_DANCE:    		
				if(robotdance_start == 1 &&  BMT_status.UI_SOC > 30)
				{    				      
				       stopcmd=0;
					dance_flag=1;
					POWER_EN();
					POWER_H_OPEN();
					POWER_V_OPEN();
					rand_num=rand_num+1;
					if(rand_num >20)rand_num=1;
					MOTOR_H_STOP_P();
					MOTOR_V_STOP_P();
					MOTOR_STOP_P();
					RobotPreAction_P(fast_temp,slow_temp);
					//aw2013_breath_all(1,1,1);
					while( !stopcmd)
					{
						switch (rand_num)
						{
						
						case 1:
							RobotDanceAction_11_P(fast_temp,slow_temp);
							break;
							
						case 2:
							RobotDanceAction_21_P(fast_temp,slow_temp);
							break;
							
						case 3:
							RobotDanceAction_31_P(fast_temp,slow_temp);
							break;
							
						case 4:
							RobotDanceAction_41_P(fast_temp,slow_temp);
							break;
						case 5:
							RobotDanceAction_12_P(fast_temp,slow_temp);
							break;
						case 6:
							RobotDanceAction_22_P(fast_temp,slow_temp);
							break;
						case 7:
							RobotDanceAction_32_P(fast_temp,slow_temp);
							break;
						case 8:
							RobotDanceAction_42_P(fast_temp,slow_temp);
							break;
						case 9:
							RobotDanceAction_13_P(fast_temp,slow_temp);
							break;
						case 10:
							RobotDanceAction_23_P(fast_temp,slow_temp);
							break;
						case 11:
							RobotDanceAction_33_P(fast_temp,slow_temp);
							break;
						case 12:
							RobotDanceAction_43_P(fast_temp,slow_temp);
							break;
						case 13:
							RobotDanceAction_14_P(fast_temp,slow_temp);
							break;
						case 14:
							RobotDanceAction_24_P(fast_temp,slow_temp);
							break;
						case 15:
							RobotDanceAction_34_P(fast_temp,slow_temp);
							break;
						case 16:
							RobotDanceAction_44_P(fast_temp,slow_temp);
							break;
						case 17:
							RobotDanceAction_15_P(fast_temp,slow_temp);
							break;
						case 18:
							RobotDanceAction_25_P(fast_temp,slow_temp);
							break;
						case 19:
							RobotDanceAction_35_P(fast_temp,slow_temp);
							break;
						case 20:
							RobotDanceAction_45_P(fast_temp,slow_temp);
							break;
						}
					}	
					
					MOTOR_HEAD_RIGHT_P(fast_temp,slow_temp);
					msleep(delaytime1000ms*2);
					if (9==limitswitch)
					{
					  MOTOR_H_STOP_P();
					  msleep(300);
	 				  MOTOR_HEAD_LEFT_P(fast_temp,slow_temp);
					  msleep(delaytime400ms*2);
					   MOTOR_H_STOP_P();
					   msleep(500);
	 				}
				}
				break;
					
			}
			     if(down ==1||up==1)
		              {
			          v_idle=0;
			   }
					
		}
	return 0;
}

static ssize_t motor_ctl_store (struct device *dev, struct device_attribute *attr,char *buf,size_t count)
{

	unsigned char pbuf[4];
	static unsigned char threadflag=0;	
	int ret=0;
	MOTOR_INIT();
	MOTOR_GPIO_POWER();

	sscanf(buf, "%3s", pbuf) ;
	
		user_cmd=pbuf[0];
		user_param=pbuf[1];
		printk("daviekuo  motor_ctl_store %c  %c\n", user_cmd, user_param);
		switch(user_cmd)
		 {
			case C_ROBOT_STOP:
				stop_flag=1;
				ret=1;
				if(robotdance_start ==0 && threadflag==1 )
				{
					threadflag=0;
					kthread_stop(thread);
				}
				if(robotdance_start ==0)
				{
					MOTOR_STOP_P();
					POWER_CLOSE();
				}
				break;
			case C_ROBOT_H_STOP:
				h_stop_flag=1;
				ret=1;
				if(robotdance_start ==0 && threadflag==1 )
				{
					threadflag=0;
					  kthread_stop(thread);
				}
				if(robotdance_start ==0)
				{
				MOTOR_H_STOP_P();
				POWER_H_CLOSE();
				}
				break;
			case C_ROBOT_STOP_DANCE:	
				robotdance_start=0;
				//aw2013_breath_all(0,0,0);
				stopcmd=1;
				ret=1;
				if( threadflag ==1 )
				{
				threadflag=0;
				 kthread_stop(thread);
				}
				robot_stop_dance();
				ret_limit=0;
			       break;
			case C_ROBOT_V_STOP:	
				ret=1;
				if(robotdance_start ==0 && threadflag==1 )
				{
					threadflag=0;
					  kthread_stop(thread);
				}
				if(robotdance_start ==0)
				{
					MOTOR_V_STOP_P();
					POWER_V_CLOSE();
				}
				break;
			case C_ROBOT_FORWORD:
			case C_ROBOT_BACK:
			case C_ROBOT_LEFT:	
			case C_ROBOT_RIGHT:	
				
			case C_ROBOT_HEAD_DOWN:			
			case C_ROBOT_HEAD_UP:			
				printk("daviekuo  in case %c  %c\n", user_cmd, user_param);

			  	ret=1;
			break;	
			case C_ROBOT_HEAD_LEFT:
				if(ret_limit ==10)ret=2; ///left  limit;
				else			  ret=1;

				//if(ret_limit ==9)ret_limit=0; ///left  limit;
				break;
			case C_ROBOT_HEAD_RIGHT:
				if(ret_limit ==9)ret=3; ///right  limit;
				else			  ret=1;

				//if(ret_limit ==10)ret_limit=0; ///left  limit;
				break;
			case C_ROBOT_START_DANCE:
				if(BMT_status.UI_SOC <= 30) //30%
				{
					 ret=2; 
					 robotdance_start=0;
					 stopcmd=1;
				}
				else  
				{
					ret=1;
					 robotdance_start=1;
				}
			break;

			default:
				 robotdance_start=0;
				 stopcmd=1;
			break;	 
			
		}
			printk("daviekuo before kthread_run %c  %c  %d  %d\n", user_cmd, user_param, ret, threadflag);		
		if(threadflag == 0 && ret == 1 && user_cmd !=C_ROBOT_V_STOP && user_cmd !=C_ROBOT_H_STOP && user_cmd !=C_ROBOT_STOP && user_cmd !=C_ROBOT_STOP_DANCE)
		{
			threadflag=1;
			printk("daviekuo  kthread_run %c  %c\n", user_cmd, user_param);
			thread = kthread_run(kkthread, 0, "dance");
			if (IS_ERR(thread)) 
			{ 
			 return PTR_ERR(thread);
			}
		}
	return ret;
}

#endif
/**************************************************************************/


static DEVICE_ATTR(motor_ctl, 0777, check_state_show, motor_ctl_store);


static struct file_operations motor_fops = {
	.owner = THIS_MODULE,
	.open = motor_open,
	.release = motor_release,
	.unlocked_ioctl = motor_unlocked_ioctl,
};

static struct miscdevice misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEV_NAME,
	.fops = &motor_fops,
};


/*----------------------------------------------------------------------------*/

static int __init motor_init(void)
{
	int err,ret;
  ret = alloc_chrdev_region(&m_dev, 0, 1, DEV_NAME);
         if (ret< 0) {
		 printk("motor  alloc_chrdev_region failed, %d", ret);
		return ret;
	}
	 motor_cdev= cdev_alloc();
	 if (motor_cdev == NULL) {
			 printk("motor cdev_alloc failed");
			 ret = -ENOMEM;
			 goto EXIT;
		 }
 	cdev_init(motor_cdev, &motor_fops);
	 motor_cdev->owner = THIS_MODULE;
	 ret = cdev_add(motor_cdev, m_dev, 1);
	 if (ret < 0) {
		  printk("Attatch file motor operation failed, %d", ret);
		 goto EXIT;
	 }
		 	 
		 motor_class = class_create(THIS_MODULE, DEV_NAME);
				 if (IS_ERR(motor_class)) {
					 printk("Failed to create class(motor)!\n");
					 return PTR_ERR(motor_class);
				 }
				 
		 motor_device = device_create(motor_class, NULL, m_dev, NULL,DEV_NAME);
		 if (IS_ERR(motor_device))
			 printk("Failed to create motor_dev device\n");

		 if (device_create_file(motor_device, &dev_attr_motor_ctl) < 0)
		         printk("Failed to create device file(%s)!\n",
					   dev_attr_motor_ctl.attr.name);	

printk("motor  init  finish--------\n");
/*
    mt_set_gpio_mode(GPIO101_PIN, GPIO_MODE_00); 
    mt_set_gpio_dir(GPIO101_PIN, GPIO_DIR_OUT) ; 
    mt_set_gpio_mode(GPIO102_PIN, GPIO_MODE_00);  
    mt_set_gpio_dir(GPIO102_PIN, GPIO_DIR_OUT) ; 
	
    mt_set_gpio_mode(GPIO43_PIN, GPIO_MODE_00);  
    mt_set_gpio_dir(GPIO43_PIN, GPIO_DIR_OUT) ; 

	POWER_EN();
    mt_set_gpio_mode(GPIO44_PIN, GPIO_MODE_00); 
    mt_set_gpio_dir(GPIO44_PIN, GPIO_DIR_OUT) ; 
    mt_set_gpio_mode(GPIO10_PIN, GPIO_MODE_00);  
    mt_set_gpio_dir(GPIO10_PIN, GPIO_DIR_OUT) ; 

    mt_set_gpio_mode(GPIO60_PIN, GPIO_MODE_00); 
    mt_set_gpio_dir(GPIO60_PIN, GPIO_DIR_OUT) ; 
    mt_set_gpio_mode(GPIO21_PIN, GPIO_MODE_00);  
    mt_set_gpio_dir(GPIO21_PIN, GPIO_DIR_OUT) ; 

	//MOTOR_FORWARD();
	MOTOR_BACK();
	//mt_set_gpio_out(GPIO43_PIN,1);
*/
/*
	MOTOR_INIT();
	POWER_EN();
	MOTOR_FORWARD_P(fast_temp,slow_temp);
*/
	return 0;

EXIT:
	 if(motor_cdev != NULL)
          {
        cdev_del(motor_cdev);
        motor_cdev = NULL;
         }

         unregister_chrdev_region(m_dev, 1);
		 
	return ret;

}
/*----------------------------------------------------------------------------*/

static void __exit motor_exit(void)
{
	  int err;
#if 1
	if(motor_cdev != NULL)
          {
	        cdev_del(motor_cdev);
	        motor_cdev = NULL;
         }
#endif
         unregister_chrdev_region(m_dev, 1);

 if(thread !=NULL)
	  kthread_stop(thread);
  return;

}

module_init(motor_init);
module_exit(motor_exit);
MODULE_AUTHOR("Yongyida");
MODULE_DESCRIPTION("motor driver");
MODULE_LICENSE("GPL");

