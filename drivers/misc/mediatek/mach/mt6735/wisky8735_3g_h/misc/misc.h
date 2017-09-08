#ifndef __MISC_H
#define __MISC_H
#ifdef __cplusplus
extern "C" {
#endif
#include <mach/gpio_const.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>

#define MISC_DEBUG

#ifdef MISC_DEBUG

#define MISC_TAG			"[misc]"
#define MISC_LOG(fmt, arg...)	printk(KERN_ERR MISC_TAG fmt, ##arg)
#define MISC_MSG(fmt, arg...)	printk(fmt, ##arg)
#define MISC_ERR(fmt, arg...)	printk(KERN_ERR MISC_TAG "ERROR,%s, %d: "fmt"\n", __FUNCTION__, __LINE__, ##arg)
#define MISC_FUNC(fmt, arg...) printk(MISC_TAG "%s\n", __FUNCTION__)

#else

#define MISC_TAG			"[misc]"
#define MISC_LOG(fmt, arg...)		//		
#define MISC_MSG(fmt, arg...)		
#define MISC_ERR(fmt, arg...)	
#define MISC_FUNC(fmt, arg...)	
#endif

//Motor
#define MOTOR_RST_PIN	(GPIO126 | 0x80000000)
#define MOTOR_BOOT0_PIN	(GPIO119 | 0x80000000)
#define MOTOR_POWEN_PIN	(GPIO127 | 0x80000000)

//Easy home   UART3  Vol converse
#define MOTOR_ZNJJPOW_PIN (GPIO2 | 0x80000000)

//charger detect
#define CHG_EN_PIN (GPIO119 | 0x80000000)
#define CHG_CTL_PIN (GPIO120 | 0x80000000)

//blue led  LDO
#define BLUE_LED_PIN (GPIO129 | 0x80000000)

//
#define LD0_3V3_PIN (GPIO127 | 0x80000000)

//UART0 TTL POW LDO
#define UART0_TTL_POW_PIN (GPIO168 | 0x80000000)
/*----------------------------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif //_CUST_EINT_H