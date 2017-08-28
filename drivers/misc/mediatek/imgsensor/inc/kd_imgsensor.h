#ifndef _KD_IMGSENSOR_H
#define _KD_IMGSENSOR_H

#include <linux/ioctl.h>
//#define CONFIG_COMPAT
#ifdef CONFIG_COMPAT
//64 bit
#include <linux/fs.h>
#include <linux/compat.h>
#endif

#ifndef ASSERT
#define ASSERT(expr)        BUG_ON(!(expr))
#endif

#define IMGSENSORMAGIC 'i'
/* IOCTRL(inode * ,file * ,cmd ,arg ) */
/* S means "set through a ptr" */
/* T means "tell by a arg value" */
/* G means "get by a ptr" */
/* Q means "get by return a value" */
/* X means "switch G and S atomically" */
/* H means "switch T and Q atomically" */

/*******************************************************************************
*
********************************************************************************/
#define YUV_INFO(_id, name, getCalData)\
    { \
    _id, name, \
    NSFeature::YUVSensorInfo <_id>::createInstance(name, #name), \
    (NSFeature::SensorInfoBase*(*)()) \
    NSFeature::YUVSensorInfo <_id>::getInstance, \
    NSFeature::YUVSensorInfo <_id>::getDefaultData, \
    getCalData, \
    NSFeature::YUVSensorInfo <_id>::getNullFlickerPara \
    }
#define RAW_INFO(_id, name, getCalData)\
    { \
    _id, name, \
    NSFeature::RAWSensorInfo <_id>::createInstance(name, #name), \
    (NSFeature::SensorInfoBase*(*)()) \
    NSFeature::RAWSensorInfo <_id>::getInstance, \
    NSFeature::RAWSensorInfo <_id>::getDefaultData, \
    getCalData, \
    NSFeature::RAWSensorInfo <_id>::getFlickerPara \
    }
/*******************************************************************************
*
********************************************************************************/

/* sensorOpen */
#define KDIMGSENSORIOC_T_OPEN                       _IO(IMGSENSORMAGIC, 0)
/* sensorGetInfo */
#define KDIMGSENSORIOC_X_GETINFO                    _IOWR(IMGSENSORMAGIC, 5, ACDK_SENSOR_GETINFO_STRUCT)
/* sensorGetResolution */
#define KDIMGSENSORIOC_X_GETRESOLUTION              _IOWR(IMGSENSORMAGIC, 10, ACDK_SENSOR_RESOLUTION_INFO_STRUCT)
/* For kernel 64-bit */
#define KDIMGSENSORIOC_X_GETRESOLUTION2             _IOWR(IMGSENSORMAGIC, 10, ACDK_SENSOR_PRESOLUTION_STRUCT)
/* sensorFeatureControl */
#define KDIMGSENSORIOC_X_FEATURECONCTROL            _IOWR(IMGSENSORMAGIC, 15, ACDK_SENSOR_FEATURECONTROL_STRUCT)
/* sensorControl */
#define KDIMGSENSORIOC_X_CONTROL                    _IOWR(IMGSENSORMAGIC, 20, ACDK_SENSOR_CONTROL_STRUCT)
/* sensorClose */
#define KDIMGSENSORIOC_T_CLOSE                      _IO(IMGSENSORMAGIC, 25)
/* sensorSearch */
#define KDIMGSENSORIOC_T_CHECK_IS_ALIVE             _IO(IMGSENSORMAGIC, 30)
/* set sensor driver */
#define KDIMGSENSORIOC_X_SET_DRIVER                 _IOWR(IMGSENSORMAGIC, 35, SENSOR_DRIVER_INDEX_STRUCT)
/* get socket postion */
#define KDIMGSENSORIOC_X_GET_SOCKET_POS             _IOWR(IMGSENSORMAGIC, 40, u32)
/* set I2C bus */
#define KDIMGSENSORIOC_X_SET_I2CBUS                 _IOWR(IMGSENSORMAGIC, 45, u32)
/* set I2C bus */
#define KDIMGSENSORIOC_X_RELEASE_I2C_TRIGGER_LOCK   _IO(IMGSENSORMAGIC, 50)
/* Set Shutter Gain Wait Done */
#define KDIMGSENSORIOC_X_SET_SHUTTER_GAIN_WAIT_DONE _IOWR(IMGSENSORMAGIC, 55, u32)
/* set mclk */
#define KDIMGSENSORIOC_X_SET_MCLK_PLL               _IOWR(IMGSENSORMAGIC, 60, ACDK_SENSOR_MCLK_STRUCT)
#define KDIMGSENSORIOC_X_GETINFO2                   _IOWR(IMGSENSORMAGIC, 65, IMAGESENSOR_GETINFO_STRUCT)
/* set open/close sensor index */
#define KDIMGSENSORIOC_X_SET_CURRENT_SENSOR         _IOWR(IMGSENSORMAGIC, 70, u32)
//set GPIO
#define KDIMGSENSORIOC_X_SET_GPIO                   _IOWR(IMGSENSORMAGIC,75,IMGSENSOR_GPIO_STRUCT)
//Get ISP CLK
#define KDIMGSENSORIOC_X_GET_ISP_CLK                _IOWR(IMGSENSORMAGIC,80,u32)

#ifdef CONFIG_COMPAT
#define COMPAT_KDIMGSENSORIOC_X_GETINFO            _IOWR(IMGSENSORMAGIC, 5, COMPAT_ACDK_SENSOR_GETINFO_STRUCT)
#define COMPAT_KDIMGSENSORIOC_X_FEATURECONCTROL    _IOWR(IMGSENSORMAGIC, 15, COMPAT_ACDK_SENSOR_FEATURECONTROL_STRUCT)
#define COMPAT_KDIMGSENSORIOC_X_CONTROL            _IOWR(IMGSENSORMAGIC, 20, COMPAT_ACDK_SENSOR_CONTROL_STRUCT)
#define COMPAT_KDIMGSENSORIOC_X_GETINFO2           _IOWR(IMGSENSORMAGIC, 65, COMPAT_IMAGESENSOR_GETINFO_STRUCT)
#define COMPAT_KDIMGSENSORIOC_X_GETRESOLUTION2     _IOWR(IMGSENSORMAGIC, 10, COMPAT_ACDK_SENSOR_PRESOLUTION_STRUCT)
#endif

/*******************************************************************************
*
********************************************************************************/
//wisky sensor 
#define GC2355F_SENSOR_ID			0x2355
#define GC2355_SENSOR_ID			0x2355
#define GC2355B_SENSOR_ID			0x2356
#define HI551_SENSOR_ID			    0x0551
#define HI841_SENSOR_ID			    0x0841
#define GC2145_SENSOR_ID                        0x2145
#define GC0310_SENSOR_ID			0xa310
#define OV8858_SENSOR_ID			  0x8858
#define OV13850MIPI_SENSOR_ID			  0xD850
#define GC0329_SENSOR_ID                         0xc0
#define GC0328_SENSOR_ID                         0x9d
#define GC5024_SENSOR_ID               		 0x5024
#define GC2755_SENSOR_ID               		 0x2655
#define GC2755_SUB_SENSOR_ID               		 0x2656
#define SP2509MIPI_SENSOR_ID			0x2509
#define SP2509_SUB_MIPI_SENSOR_ID			0x250a
#define GC0409MIPI_SENSOR_ID 0x0409
#define SP0A09MIPI_SENSOR_ID										0x0a09
#define HI545MIPI_SENSOR_ID	0x2555
#define GC5005MIPI_SENSOR_ID               		 0x5005
#define OV5695MIPI_SENSOR_ID                    0x5695
#define OV8856_SENSOR_ID						0x885a
#define OV5648MIPI_SENSOR_ID						0x5648
#define S5K3H5XA_SENSOR_ID 				0x3085
/* CAMERA DRIVER NAME */
#define CAMERA_HW_DEVNAME            "kd_camera_hw"

#define SENSOR_DRVNAME_GC2355F_MIPI_RAW   "gc2355fmipiraw"
#define SENSOR_DRVNAME_GC2355_MIPI_RAW   "gc2355mipiraw"
#define SENSOR_DRVNAME_SP2509_MIPI_RAW  "sp2509mipiraw"
#define SENSOR_DRVNAME_SP2509_SUB_MIPI_RAW  "sp2509submipiraw"
#define SENSOR_DRVNAME_GC0409_MIPI_RAW   "gc0409mipiraw"
#define SENSOR_DRVNAME_HI545_MIPI_RAW   "hi545mipiraw"
#define SENSOR_DRVNAME_GC2755_MIPI_RAW   "gc2755mipiraw"
#define SENSOR_DRVNAME_GC2755_SUB_MIPI_RAW   "gc2755submipiraw"
#define SENSOR_DRVNAME_GC2355B_MIPI_RAW   "gc2355bmipiraw"
#define SENSOR_DRVNAME_HI551_MIPI_RAW   "hi551mipiraw"
#define SENSOR_DRVNAME_HI841_MIPI_RAW   "hi841mipiraw"
#define SENSOR_DRVNAME_GC2145_MIPI_YUV          "gc2145mipiyuv"
#define SENSOR_DRVNAME_GC0310_MIPI_YUV   "gc0310mipiyuv"
#define SENSOR_DRVNAME_OV8858_MIPI_RAW   "ov8858mipiraw"
#define SENSOR_DRVNAME_OV13850_MIPI_RAW   "ov13850mipiraw"
#define SENSOR_DRVNAME_GC0329_YUV   "gc0329_yuv"
#define SENSOR_DRVNAME_GC0328_YUV   "gc0328_yuv"
#define SENSOR_DRVNAME_SP0A09_MIPI_RAW          "sp0a09mipiraw"
#define SENSOR_DRVNAME_GC5024_MIPI_RAW          "gc5024mipiraw"
#define SENSOR_DRVNAME_HI545_MIPI_RAW               "hi545mipiraw"
#define SENSOR_DRVNAME_OV2680_MIPI_RAW  "ov2680mipiraw"
#define SENSOR_DRVNAME_GC5005_MIPI_RAW   "gc5005mipiraw"
#define SENSOR_DRVNAME_OV5695_MIPI_RAW          "ov5695mipi"
#define SENSOR_DRVNAME_OV8856_MIPI_RAW          "ov8856mipi"
#define SENSOR_DRVNAME_OV5648_MIPI_RAW   	"ov5648mipi"
#define SENSOR_DRVNAME_S5K3H5XA_MIPI_RAW  "s5k3h5xamipiraw"
/*******************************************************************************
*
********************************************************************************/
void KD_IMGSENSOR_PROFILE_INIT(void);
void KD_IMGSENSOR_PROFILE(char *tag);

#define mDELAY(ms)     mdelay(ms)
#define uDELAY(us)       udelay(us)
#endif              /* _KD_IMGSENSOR_H */
