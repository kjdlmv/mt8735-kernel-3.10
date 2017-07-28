
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
#define MOTOR_RST_PIN	(GPIO102 | 0x80000000)
#define MOTOR_BOOT0_PIN	(GPIO119 | 0x80000000)
#define MOTOR_POWEN_PIN	(GPIO101 | 0x80000000)
/*
//Easy home
#define MOTOR_ZNJJPOW_PIN (GPIO2 | 0x80000000)
*/
//ircam
#define IRCAM_PWREN_PIN (GPIO100 | 0x80000000)

//charger detect
#define CHG_DET_EN_PIN (GPIO98 | 0x80000000)
#define CHG_DET_PIN (GPIO83 | 0x80000000)

/*----------------------------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif //_CUST_EINT_H

