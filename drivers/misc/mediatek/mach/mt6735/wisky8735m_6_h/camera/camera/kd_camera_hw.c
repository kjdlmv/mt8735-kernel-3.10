#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <linux/kernel.h>


#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"


//dongkang@wisky.com.cn,2014-12-13
//#define DBG_CAM  printk
#define DBG_CAM(x...)

bool _hwPowerOn(MT65XX_POWER powerId, int powerVolt, char *mode_name);
bool _hwPowerDown(MT65XX_POWER powerId, char *mode_name);

#define SENSOR_IS(sname,cname) \
    (0 == strcmp(sname,cname))
     

#define SET_GPIO_RST(pin,high,sensor) \
    do{ \
    	  if(mt_set_gpio_mode(pinSet[pin][T_CMRST],pinSet[pin][T_CMRST+T_MODE])) \
        { \
        	DBG_CAM("%s:set gpio rst(%s) mode failed\n",__func__,sensor); \
        	goto ERR_OUT; \
        } \
        if(mt_set_gpio_dir(pinSet[pin][T_CMRST],GPIO_DIR_OUT)) \
        { \
        	DBG_CAM("%s:set gpio rst(%s) dir failed\n",__func__,sensor); \
          goto ERR_OUT; \
        } \
        if(mt_set_gpio_out(pinSet[pin][T_CMRST],(high) ? GPIO_HIGH : GPIO_LOW)) \
        { \
        	DBG_CAM("%s:set gpio rst(%s) as %d failed\n",__func__,high); \
        	goto ERR_OUT; \
        } \
    }while(0)

 
#define SET_GPIO_PDN(pin,high,sensor) \
    do{ \
    	  if(mt_set_gpio_mode(pinSet[pin][T_CMPDN],pinSet[pin][T_CMPDN+T_MODE])) \
        { \
        	DBG_CAM("%s:set gpio rst(%s) mode failed\n",__func__,sensor); \
        	goto ERR_OUT; \
        } \
        if(mt_set_gpio_dir(pinSet[pin][T_CMPDN],GPIO_DIR_OUT)) \
        { \
        	DBG_CAM("%s:set gpio rst(%s) dir failed\n",__func__,sensor); \
          goto ERR_OUT; \
        } \
        if(mt_set_gpio_out(pinSet[pin][T_CMPDN],(high) ? GPIO_HIGH : GPIO_LOW)) \
        { \
        	DBG_CAM("%s:set gpio rst(%s) as %d failed\n",__func__,high); \
        	goto ERR_OUT; \
        } \
    }while(0)
    

#define POWER_ON(pin,vol,name,info) \
   do{ \
       if(TRUE != _hwPowerOn(pin, vol, name)) \
       { \
		      DBG_CAM("#################%s:POWERON failed###################\n",info); \
		      /*goto ERR_OUT;*/ \
		   } \
   }while(0)
   
#define POWER_OFF(pin,name,info) \
   do{ \
      if(TRUE != _hwPowerDown(pin,name)) \
      { \
      	 DBG_CAM("#################%s:POWEROFFfailed####################\n",info); \
		  } \
   }while(0)
   
// 4G 机器前后置共用 MCLK，因此只需要控制 MCLK1 即可   
#define MCLK_ON(idx) \
   do{ \
   	   ISP_MCLK1_EN(1); \
   }while(0)   

#define MCLK_OFF(idx) \
   do{ \
  	   ISP_MCLK1_EN(0); \
   }while(0)   


#define T_CMRST 0
#define T_MODE  1
#define T_CMPDN 2

#define GPIO_LOW   GPIO_OUT_ZERO
#define GPIO_HIGH  GPIO_OUT_ONE
static DEFINE_SPINLOCK(kdsensor_pw_cnt_lock);
int cntVCAMD =0;
int cntVCAMA =0;
int cntVCAMIO =0;
int cntVCAMAF =0;
int cntVCAMD_SUB =0;


static u32 pinSet[2][4] = {
			//for main sensor 
			{
				GPIO_CAMERA_CMRST_PIN,
				GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
				GPIO_CAMERA_CMPDN_PIN,
				GPIO_CAMERA_CMPDN_PIN_M_GPIO,
			},
			// for sub sensor
			{
				GPIO_CAMERA_CMRST1_PIN,
				GPIO_CAMERA_CMRST1_PIN_M_GPIO,
				GPIO_CAMERA_CMPDN1_PIN,
				GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
			},
};

bool _hwPowerOn(MT65XX_POWER powerId, int powerVolt, char *mode_name){

	if( hwPowerOn( powerId,  powerVolt, mode_name))
	{
	    spin_lock(&kdsensor_pw_cnt_lock);
		if(powerId==CAMERA_POWER_VCAM_D)
			cntVCAMD+= 1;
		else if(powerId==CAMERA_POWER_VCAM_A)
			cntVCAMA+= 1;
		else if(powerId==CAMERA_POWER_VCAM_IO)
			cntVCAMIO+= 1;
		else if(powerId==CAMERA_POWER_VCAM_AF)
			cntVCAMAF+= 1;
		else if(powerId==SUB_CAMERA_POWER_VCAM_D)
			cntVCAMD_SUB+= 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

bool _hwPowerDown(MT65XX_POWER powerId, char *mode_name){

	if( hwPowerDown( powerId, mode_name))
	{
	    spin_lock(&kdsensor_pw_cnt_lock);
		if(powerId==CAMERA_POWER_VCAM_D)
			cntVCAMD-= 1;
		else if(powerId==CAMERA_POWER_VCAM_A)
			cntVCAMA-= 1;
		else if(powerId==CAMERA_POWER_VCAM_IO)
			cntVCAMIO-= 1;
		else if(powerId==CAMERA_POWER_VCAM_AF)
			cntVCAMAF-= 1;
		else if(powerId==SUB_CAMERA_POWER_VCAM_D)
			cntVCAMD_SUB-= 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

void checkPowerBeforClose( char* mode_name)
{

	int i= 0;

	DBG_CAM("[checkPowerBeforClose]cntVCAMD:%d, cntVCAMA:%d,cntVCAMIO:%d, cntVCAMAF:%d, cntVCAMD_SUB:%d,\n",
		cntVCAMD, cntVCAMA,cntVCAMIO,cntVCAMAF,cntVCAMD_SUB);


	for(i=0;i<cntVCAMD;i++)
		hwPowerDown(CAMERA_POWER_VCAM_D,mode_name);
	for(i=0;i<cntVCAMA;i++)
		hwPowerDown(CAMERA_POWER_VCAM_A,mode_name);
	for(i=0;i<cntVCAMIO;i++)
		hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name);
	for(i=0;i<cntVCAMAF;i++)
		hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name);
	for(i=0;i<cntVCAMD_SUB;i++)
		hwPowerDown(SUB_CAMERA_POWER_VCAM_D,mode_name);

	 cntVCAMD =0;
	 cntVCAMA =0;
	 cntVCAMIO =0;
	 cntVCAMAF =0;
	 cntVCAMD_SUB =0;

}
extern void ISP_MCLK1_EN(BOOL En);
//extern void ISP_MCLK2_EN(BOOL En);

// 以下上电时序严格按照datasheet的顺序，如出现概率性打不开或黑屏，
// 请参照最新datasheet，检查上电时序，增加或减少delay
// 2015-1-19, dongkang@wisky.com.cn
static int sensor_gc0310_mipiyuv_poweron_seq(int pin_idx,char *mode_name,char *sensor){
  DBG_CAM("%s\n",__func__);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(10);
   // IOVDD
   POWER_ON(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name,"IOVDD18");
   mdelay(50);
   // AVDD
   POWER_ON(CAMERA_POWER_VCAM_A, VOL_2800,mode_name,"AVDD28");
	 mdelay(10);
	 
	 MCLK_ON(pin_idx);
	 mdelay(10);
	 // PDN High
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(20);
   // PDN Low 
   SET_GPIO_PDN(pin_idx,0,sensor);
	 // RST HIGH
	 SET_GPIO_RST(pin_idx,1,sensor);
	 mdelay(1); 
	 
	 return 0;

ERR_OUT: // this must be defiined !!
	 return -EIO;
}

static int sensor_gc0310_mipiyuv_poweroff_seq(int pin_idx,char *mode_name,char *sensor){
  DBG_CAM("%s\n",__func__);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(10);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(10);
   
   MCLK_OFF(pin_idx);
   mdelay(10);
   
ERR_OUT: 
   //AVDD
   POWER_OFF(CAMERA_POWER_VCAM_A,mode_name,"AVDD");
   //IOVDD
   POWER_OFF(CAMERA_POWER_VCAM_IO,mode_name,"IOVDD");
   
   return 0;
}

static int sensor_gc0329_yuv_poweron_seq(int pin_idx,char *mode_name,char *sensor){
  DBG_CAM("%s\n",__func__);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(10);
   // IOVDD
   POWER_ON(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name,"IOVDD18");
   mdelay(50);
   // AVDD
   POWER_ON(CAMERA_POWER_VCAM_A, VOL_2800,mode_name,"AVDD28");
	 mdelay(10);
	 
	 MCLK_ON(pin_idx);
	 mdelay(10);
	 // PDN High
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(20);
   // PDN Low 
   SET_GPIO_PDN(pin_idx,0,sensor);
	 // RST HIGH
	 SET_GPIO_RST(pin_idx,1,sensor);
	 mdelay(1); 
	 
	 return 0;

ERR_OUT: // this must be defiined !!
	 return -EIO;
}

static int sensor_gc0329_yuv_poweroff_seq(int pin_idx,char *mode_name,char *sensor){
  DBG_CAM("%s\n",__func__);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(10);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(10);
   
   MCLK_OFF(pin_idx);
   mdelay(10);
   
ERR_OUT: 
   //AVDD
   POWER_OFF(CAMERA_POWER_VCAM_A,mode_name,"AVDD");
   //IOVDD
   POWER_OFF(CAMERA_POWER_VCAM_IO,mode_name,"IOVDD");
   
   return 0;
}

static int sensor_gc0328_yuv_poweron_seq(int pin_idx,char *mode_name,char *sensor){
  DBG_CAM("%s\n",__func__);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(10);
   // IOVDD
   POWER_ON(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name,"IOVDD18");
   mdelay(50);
   // AVDD
   POWER_ON(CAMERA_POWER_VCAM_A, VOL_2800,mode_name,"AVDD28");
	 mdelay(10);
	 
	 MCLK_ON(pin_idx);
	 mdelay(10);
	 // PDN High
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(20);
   // PDN Low 
   SET_GPIO_PDN(pin_idx,0,sensor);
	 // RST HIGH
	 SET_GPIO_RST(pin_idx,1,sensor);
	 mdelay(1); 
	 
	 return 0;

ERR_OUT: // this must be defiined !!
	 return -EIO;
}

static int sensor_gc0328_yuv_poweroff_seq(int pin_idx,char *mode_name,char *sensor){
  DBG_CAM("%s\n",__func__);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(10);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(10);
   
   MCLK_OFF(pin_idx);
   mdelay(10);
   
ERR_OUT: 
   //AVDD
   POWER_OFF(CAMERA_POWER_VCAM_A,mode_name,"AVDD");
   //IOVDD
   POWER_OFF(CAMERA_POWER_VCAM_IO,mode_name,"IOVDD");
   
   return 0;
}


static int sensor_gc2355_mipiraw_poweron_seq(int pin_idx,char *mode_name,char *sensor){
   DBG_CAM("%s\n",__func__);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(10);
   // IOVDD
   POWER_ON(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name,"IOVDD18");
   mdelay(100);
   // DVDD
	 POWER_ON(CAMERA_POWER_VCAM_D, VOL_1500,mode_name,"DVDD15"); // 4G机器在硬件上不支持1.8V的DVDD
	 mdelay(50);
	 // AVDD
   POWER_ON(CAMERA_POWER_VCAM_A, VOL_2800,mode_name,"AVDD28");
	 mdelay(50);
	 
	 MCLK_ON(pin_idx);
   mdelay(10);
	 // PDN LOW
   SET_GPIO_PDN(pin_idx,0,sensor);
   mdelay(10);
	 // RST HIGH
	 SET_GPIO_RST(pin_idx,1,sensor);
	 mdelay(1); 
	 
	 return 0;

ERR_OUT:
	 return -EIO;
}

static int sensor_gc2355_mipiraw_poweroff_seq(int pin_idx,char *mode_name,char *sensor){
   DBG_CAM("%s\n",__func__);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(10);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(10);
   
   MCLK_OFF(pin_idx);
   mdelay(10);
   
ERR_OUT: 
   //AVDD
   POWER_OFF(CAMERA_POWER_VCAM_A,mode_name,"AVDD");
   mdelay(1);
   //DVDD
   POWER_OFF(CAMERA_POWER_VCAM_D,mode_name,"DVDD");
   mdelay(1);
   //IOVDD
   POWER_OFF(CAMERA_POWER_VCAM_IO,mode_name,"IOVDD");
   mdelay(1);
   
   return 0;
}

static int sensor_hi551_mipiraw_poweron_seq(int pin_idx,char *mode_name,char *sensor){
	 DBG_CAM("%s\n",__func__);
   
   MCLK_ON(pin_idx);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   //SET_GPIO_RST(1-pin_idx,0,sensor); //gc2355
   
   mdelay(1);
   // PDN LOW
   SET_GPIO_PDN(pin_idx,0,sensor);
   //SET_GPIO_PDN(1-pin_idx,1,sensor); //gc2355
   
   mdelay(1);
   // IOVDD
	 POWER_ON(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name,"IOVDD18");  //1.8V 或 2.8V
	 mdelay(1);
	 // AVDD28
   POWER_ON(CAMERA_POWER_VCAM_A, VOL_2800,mode_name,"AVDD28");  
	 mdelay(1);
	 // DVDD
	 POWER_ON(CAMERA_POWER_VCAM_D, VOL_1200,mode_name,"DVDD12");  //1.8V 或 1.2V
	 mdelay(5);
	 
	 //AF_VCC
	 POWER_ON(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name,"AFVDD28");  
	 mdelay(1);
   
    // RST HIGH
   SET_GPIO_RST(pin_idx,1,sensor);
   mdelay(2);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(20);
   
   //while(1);//for test vol on hardware
      
   return 0;

   
ERR_OUT:
	 return -EIO;
}

static int sensor_hi551_mipiraw_poweroff_seq(int pin_idx,char *mode_name, char *sensor){
   
   DBG_CAM("%s\n",__func__);

   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   mdelay(1);
   // PDN LOW
   SET_GPIO_PDN(pin_idx,0,sensor);
   mdelay(1);
   
   MCLK_OFF(pin_idx);
   mdelay(1);
   
ERR_OUT:
   //DVDD
   POWER_OFF(CAMERA_POWER_VCAM_D,mode_name,"DVDD");
   mdelay(1);
   //AVDD
   POWER_OFF(CAMERA_POWER_VCAM_A,mode_name,"AVDD");
   mdelay(1);
   //IOVDD
   POWER_OFF(CAMERA_POWER_VCAM_IO,mode_name,"IOVDD");
   mdelay(1);
   //AFVDD
   POWER_OFF(CAMERA_POWER_VCAM_AF,mode_name,"AFVDD");
   mdelay(1);
   
   return 0;
}


static int sensor_common_poweron_seq(int pin_idx,char *mode_name,char *sensor){
	 DBG_CAM("%s\n",__func__);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   // PDN LOW
   SET_GPIO_PDN(pin_idx,0,sensor);
   mdelay(50);
   // AVDD28
   POWER_ON(CAMERA_POWER_VCAM_A, VOL_2800,mode_name,"AVDD28");  
	 mdelay(10);
	 // IOVDD
	 POWER_ON(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name,"IOVDD18");  
	 mdelay(10);
	 
	 // DVDD
	 POWER_ON(CAMERA_POWER_VCAM_D, VOL_1200,mode_name,"DVDD12");  
	 mdelay(10);

	 //AF_VCC
	 POWER_ON(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name,"AFVDD28");  
	 mdelay(50);
	 
	 MCLK_ON(pin_idx);
   mdelay(10);
   
   // RST HIGH
   SET_GPIO_RST(pin_idx,1,sensor);
   mdelay(5);
   // PDN HIGH
   SET_GPIO_PDN(pin_idx,1,sensor);
   mdelay(5);
   
   return 0;
   
ERR_OUT:
	 return -EIO;
}

static int sensor_common_poweroff_seq(int pin_idx,char *mode_name, char *sensor){
   
   DBG_CAM("%s\n",__func__);
   // RST LOW
   SET_GPIO_RST(pin_idx,0,sensor);
   // PDN LOW
   SET_GPIO_PDN(pin_idx,0,sensor);
   mdelay(5);
   
   MCLK_OFF(pin_idx);
   mdelay(10);
   
ERR_OUT:
   //DVDD
   POWER_OFF(CAMERA_POWER_VCAM_D,mode_name,"DVDD");
   //IOVDD
   POWER_OFF(CAMERA_POWER_VCAM_IO,mode_name,"IOVDD");
   //AVDD
   POWER_OFF(CAMERA_POWER_VCAM_A,mode_name,"AVDD");
   //AFVDD
   POWER_OFF(CAMERA_POWER_VCAM_AF,mode_name,"AFVDD");
   
   return 0;
   
}

static int disable_another_sensor(int pin_idx, char *sensor){
	 // RST LOW
   SET_GPIO_RST(1-pin_idx,0,sensor);
   //PDN LOW
   SET_GPIO_PDN(1-pin_idx,0,sensor);
   mdelay(1);
	
ERR_OUT:
	 return 0;
}


int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char *mode_name){
	u32 pin_idx = 0;
	
	 DBG_CAM("#########CAMERA_POWER_%d(%s)############\n",On,currSensorName);
 	
	// Main & Sub camera share the same bus
	if ((DUAL_CAMERA_MAIN_SENSOR == SensorIdx) && \
		  currSensorName && \
		  (SENSOR_IS(SENSOR_DRVNAME_HI551_MIPI_RAW,  currSensorName) || \
		   SENSOR_IS(SENSOR_DRVNAME_GC2355B_MIPI_RAW, currSensorName)))
  {
		pin_idx = 0;
	}
  else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx && \
		       currSensorName && \
		      (SENSOR_IS(SENSOR_DRVNAME_GC2355F_MIPI_RAW, currSensorName)|| \
					 SENSOR_IS(SENSOR_DRVNAME_GC0329_YUV, currSensorName)|| \
					 SENSOR_IS(SENSOR_DRVNAME_GC0328_YUV, currSensorName)|| \
		       SENSOR_IS(SENSOR_DRVNAME_GC0310_MIPI_YUV, currSensorName)))
	{ 
		pin_idx = 1;
	}
  else
	{
		DBG_CAM("Not match(%s AS %d(1:Main,2:Sub))\n",currSensorName,SensorIdx);
		goto ERR_OUT;
	}
  
	if(On){  //power on
		  
		  // 4G机器不支持PIP功能，硬件上没有区分前后置的DVDD，故同时打开可能会有问题
		  // 在不同时打开摄像头时，打开一个就禁止另一个，避免可能的相互干扰
		  // disable another sensor first
		  //disable_another_sensor(pin_idx,currSensorName);
		
      if(SENSOR_IS(SENSOR_DRVNAME_GC0310_MIPI_YUV, currSensorName))
		     return sensor_gc0310_mipiyuv_poweron_seq(pin_idx,mode_name,currSensorName);
		  else if(SENSOR_IS(SENSOR_DRVNAME_GC2355B_MIPI_RAW, currSensorName) || SENSOR_IS(SENSOR_DRVNAME_GC2355F_MIPI_RAW, currSensorName))
		     return sensor_gc2355_mipiraw_poweron_seq(pin_idx,mode_name,currSensorName);
		  else if(SENSOR_IS(SENSOR_DRVNAME_HI551_MIPI_RAW,  currSensorName))
		  	 return sensor_hi551_mipiraw_poweron_seq(pin_idx,mode_name,currSensorName);
		  else if(SENSOR_IS(SENSOR_DRVNAME_GC0329_YUV, currSensorName))
		  	 return sensor_gc0329_yuv_poweron_seq(pin_idx,mode_name,currSensorName);
		  else if(SENSOR_IS(SENSOR_DRVNAME_GC0328_YUV, currSensorName))
		  	 return sensor_gc0328_yuv_poweron_seq(pin_idx,mode_name,currSensorName);	 
		  else
		  	 return sensor_common_poweron_seq(pin_idx,mode_name,currSensorName);
	
	}
	else{ // power off
	
	  if(SENSOR_IS(SENSOR_DRVNAME_GC0310_MIPI_YUV, currSensorName))
	  	  return sensor_gc0310_mipiyuv_poweroff_seq(pin_idx,mode_name,currSensorName); 
	  else if(SENSOR_IS(SENSOR_DRVNAME_GC2355B_MIPI_RAW, currSensorName) || SENSOR_IS(SENSOR_DRVNAME_GC2355F_MIPI_RAW, currSensorName))
    	   return sensor_gc2355_mipiraw_poweroff_seq(pin_idx,mode_name,currSensorName); 
    else if(SENSOR_IS(SENSOR_DRVNAME_HI551_MIPI_RAW,  currSensorName))
		  	 return sensor_hi551_mipiraw_poweroff_seq(pin_idx,mode_name,currSensorName);
		else if(SENSOR_IS(SENSOR_DRVNAME_GC0329_YUV, currSensorName))
		  	 return sensor_gc0329_yuv_poweroff_seq(pin_idx,mode_name,currSensorName);  	 
		else if(SENSOR_IS(SENSOR_DRVNAME_GC0328_YUV, currSensorName))
		  	 return sensor_gc0328_yuv_poweroff_seq(pin_idx,mode_name,currSensorName);  	  
		else  
	       return sensor_common_poweroff_seq(pin_idx,mode_name,currSensorName);
  }

ERR_OUT:
    return -EIO;
}

EXPORT_SYMBOL(kdCISModulePowerOn);



