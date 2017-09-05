#include <mach/mt_typedefs.h>
#include <mach/mt_gpt.h>
#include <mach/mt_boot.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>




#define  POWER_LED     (GPIO124|0x80000000)
#define AW3215_EN      (GPIO119|0x80000000)




void close_charger_led(void)
{
	mt_set_gpio_mode(POWER_LED, GPIO_MODE_00);  
	 mt_set_gpio_dir(POWER_LED, GPIO_DIR_OUT) ;	
	 mt_set_gpio_out(POWER_LED, 1);
}
void open_charger_led(void)
{
	mt_set_gpio_mode(POWER_LED, GPIO_MODE_00);  
	 mt_set_gpio_dir(POWER_LED, GPIO_DIR_OUT) ;	
	 mt_set_gpio_out(POWER_LED, 0);
}
void EN_charger_IC(void)
{
	mt_set_gpio_mode(AW3215_EN, GPIO_MODE_00);  
	 mt_set_gpio_dir(AW3215_EN, GPIO_DIR_OUT) ;	
	 mt_set_gpio_out(AW3215_EN, 0);
}
void DIS_charger_IC(void)
{
	mt_set_gpio_mode(AW3215_EN, GPIO_MODE_00);  
	 mt_set_gpio_dir(AW3215_EN, GPIO_DIR_OUT) ;	
	 mt_set_gpio_out(AW3215_EN, 1);
}


