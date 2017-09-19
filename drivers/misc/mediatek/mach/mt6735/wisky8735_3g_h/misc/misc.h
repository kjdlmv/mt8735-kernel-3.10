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

//Motor
#define MOTOR_RST_PIN	(GPIO44 | 0x80000000)
#define MOTOR_BOOT0_PIN	(GPIO42 | 0x80000000)
#define MOTOR_POWEN_PIN	(GPIO43 | 0x80000000)
//Easy home
#define MOTOR_ZNJJPOW_PIN (GPIO2 | 0x80000000)
//Charger
#define CHG_USB_EN	(GPIO88 | 0x80000000)
#define CHG_EN_PIN (GPIO119 | 0x80000000)
#define CHG_CTL_PIN (GPIO120 | 0x80000000)

extern kal_bool gsensor_data_switch;
/*----------------------------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif //_CUST_EINT_H