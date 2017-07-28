//s_add new sensor driver here
//export funtions

//wisky sensor
UINT32 GC2355F_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC2355B_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 OV5648MIPISensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC0310_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 HI551_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 HI841_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 OV13850_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 OV8858_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC0329_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC0328_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 GC5024MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
UINT32 SP0A09MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc);
//! Add Sensor Init function here
//! Note:
//! 1. Add by the resolution from ""large to small"", due to large sensor
//!    will be possible to be main sensor.
//!    This can avoid I2C error during searching sensor.
//! 2. This file should be the same as mediatek\custom\common\hal\imgsensor\src\sensorlist.cpp
ACDK_KD_SENSOR_INIT_FUNCTION_STRUCT kdSensorList[MAX_NUM_OF_SUPPORT_SENSOR+1] =
{
// wisky sensor
#if defined(GC2355F_MIPI_RAW)
    {GC2355F_SENSOR_ID, SENSOR_DRVNAME_GC2355F_MIPI_RAW,GC2355F_MIPI_RAW_SensorInit}, 
#endif
#if defined(GC2355B_MIPI_RAW)
    {GC2355B_SENSOR_ID, SENSOR_DRVNAME_GC2355B_MIPI_RAW,GC2355B_MIPI_RAW_SensorInit}, 
#endif
#if defined(GC5024_MIPI_RAW)
		{GC5024_SENSOR_ID, SENSOR_DRVNAME_GC5024_MIPI_RAW, GC5024MIPI_RAW_SensorInit},
#endif
#if defined(SP0A09_MIPI_RAW)
		{SP0A09_SENSOR_ID, SENSOR_DRVNAME_SP0A09_MIPI_RAW, SP0A09MIPI_RAW_SensorInit},
#endif
#if defined(GC0310_MIPI_YUV)
    {GC0310_SENSOR_ID, SENSOR_DRVNAME_GC0310_MIPI_YUV,GC0310_YUV_SensorInit},
#endif
#if defined(HI551_MIPI_RAW)
    {HI551_SENSOR_ID, SENSOR_DRVNAME_HI551_MIPI_RAW,HI551_MIPI_RAW_SensorInit}, 
#endif
#if defined(GC0328_YUV)
{GC0328_SENSOR_ID,SENSOR_DRVNAME_GC0328_YUV,GC0328_YUV_SensorInit},
#endif
#if defined(GC0329_YUV)
{GC0329_SENSOR_ID,SENSOR_DRVNAME_GC0329_YUV,GC0329_YUV_SensorInit},
#endif

/*  ADD sensor driver before this line */
    {0,{0},NULL}, //end of list
};
//e_add new sensor driver here

