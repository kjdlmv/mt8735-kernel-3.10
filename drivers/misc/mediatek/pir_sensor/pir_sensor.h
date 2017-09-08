#ifndef PIR_SENSOR_H__
#define PIR_SENSOR_H__

#define PIR_SENSOR_DEBUG

#ifdef PIR_SENSOR_DEBUG

#define PS_TAG			"[pir_sensor]"
#define PS_LOG(fmt, arg...)	printk(KERN_ERR PS_TAG fmt, ##arg)
#define PS_MSG(fmt, arg...)	printk(fmt, ##arg)
#define PS_ERR(fmt, arg...)	printk(KERN_ERR PS_TAG "ERROR,%s, %d: "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#define PS_FUNC(fmt, arg...) printk(PS_TAG "%s\n", __FUNCTION__)

#else

#define PS_TAG			"[pir_sensor]"
#define PS_LOG(fmt, arg...)		//		
#define PS_MSG(fmt, arg...)		
#define PS_ERR(fmt, arg...)	
#define PS_FUNC(fmt, arg...)	
#endif


#define PIR_SENSOR_PWEREN_PIN		(GPIO67 | 0x80000000)
#define PIR_SENSOR_EINT_PIN		(GPIO0 | 0x80000000)


#endif /* PIR_SENSOR_H__ */
