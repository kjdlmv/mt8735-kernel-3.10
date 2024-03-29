#ifndef _CUST_PMIC_H_
#define _CUST_PMIC_H_

//#define PMIC_VDVFS_CUST_ENABLE

#define LOW_POWER_LIMIT_LEVEL_1 15

//Define for disable low battery protect feature, default no define for enable low battery protect.
//#define DISABLE_LOW_BATTERY_PROTECT    
                                         
//Define for disable battery OC protect
//#define DISABLE_BATTERY_OC_PROTECT     
                                         
//Define for disable battery 15% protect 
//#define DISABLE_BATTERY_PERCENT_PROTECT

//Define for DLPT
#define DISABLE_DLPT_FEATURE
#define POWER_UVLO_VOLT_LEVEL 2600
#define IMAX_MAX_VALUE 5500//mA

#define POWER_INT0_VOLT 3400
#define POWER_INT1_VOLT 3250
#define POWER_INT2_VOLT 3000

#if defined(CONFIG_ARCH_MT6753)
#define POWER_BAT_OC_CURRENT_H    4670
#define POWER_BAT_OC_CURRENT_L    5500
#define POWER_BAT_OC_CURRENT_H_RE 4670 //3400
#define POWER_BAT_OC_CURRENT_L_RE 5500 //4000
#else
#define POWER_BAT_OC_CURRENT_H    3400
#define POWER_BAT_OC_CURRENT_L    4000
#define POWER_BAT_OC_CURRENT_H_RE 3400 //3400
#define POWER_BAT_OC_CURRENT_L_RE 4000 //4000
#endif

#define DLPT_POWER_OFF_EN
#define POWEROFF_BAT_CURRENT 3000//mA
#define DLPT_POWER_OFF_THD 50

//#define BATTERY_MODULE_INIT

#if defined(CONFIG_MTK_BQ24196_SUPPORT)\
	||defined(CONFIG_MTK_BQ24296_SUPPORT)\
	||defined(CONFIG_MTK_BQ24160_SUPPORT)\
	||defined(CONFIG_MTK_BQ24261_SUPPORT)\
	||defined(CONFIG_MTK_NCP1854_SUPPORT) \
	||defined(CONFIG_MTK_ETA6005_SUPPORT)

#define SWCHR_POWER_PATH
#endif

#if defined(CONFIG_MTK_FAN5402_SUPPORT) \
	 || defined(CONFIG_MTK_FAN5405_SUPPORT) \
	  || defined(CONFIG_MTK_BQ24158_SUPPORT) \
	   || defined(CONFIG_MTK_BQ24196_SUPPORT) \
	    || defined(CONFIG_MTK_BQ24296_SUPPORT) \
	     || defined(CONFIG_MTK_NCP1851_SUPPORT) \
	      || defined(CONFIG_MTK_NCP1854_SUPPORT) \
	       || defined(CONFIG_MTK_BQ24160_SUPPORT) \
	        || defined(CONFIG_MTK_BQ24157_SUPPORT) \
	         || defined(CONFIG_MTK_BQ24250_SUPPORT) \
	          || defined(CONFIG_MTK_BQ24261_SUPPORT) \
	           ||defined(CONFIG_MTK_ETA6005_SUPPORT) 
#define EXTERNAL_SWCHR_SUPPORT
#endif

/* ADC Channel Number */
typedef enum {
	//MT6325
	AUX_BATSNS_AP =		0x000,
	AUX_ISENSE_AP,
	AUX_VCDT_AP,
	AUX_BATON_AP,
	AUX_TSENSE_AP,
	AUX_TSENSE_MD =		0x005,
	AUX_VACCDET_AP =	0x007,
	AUX_VISMPS_AP =		0x00B,
	AUX_ICLASSAB_AP =	0x016,
	AUX_HP_AP =		0x017,
	AUX_CH10_AP =		0x018,
	AUX_VBIF_AP =		0x019,
	
	AUX_CH0_6311 =		0x020,
	AUX_CH1_6311 =		0x021,

	AUX_ADCVIN0_MD =	0x10F,
	AUX_ADCVIN0_GPS = 	0x20C,
	AUX_CH12 = 		0x1011,
	AUX_CH13 = 		0x2011,
	AUX_CH14 = 		0x3011,
	AUX_CH15 = 		0x4011,
} upmu_adc_chl_list_enum;

#endif /* _CUST_PMIC_H_ */ 
