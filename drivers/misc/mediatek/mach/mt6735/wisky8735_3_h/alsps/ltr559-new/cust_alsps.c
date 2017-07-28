#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
#include <mach/upmu_common.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
     .als_level  =   {3,  10,  30, 70,  300,   600,    1000, 	1500, 	 2500, 	4000, 5400,	 12000,  35000,  50000,  65535},	/* als_code */    
     .als_value  =   {0,  10,  40, 180, 300,   500,    950,   1200,    1800,  3600, 4500,  6000,   7000,   7000,  8000,10000}, 
    .ps_threshold_high = 500,  //1500,   // 1800,  // 1345,	 //838, //700,	//430,
    .ps_threshold_low = 200,  //1500,  // 900, //392, //500,	//250,
    .ps_threshold = 900,
    .is_batch_supported_ps = false,
    .is_batch_supported_als = false,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

