#ifndef MISC_H_
#define MISC_H_

#define MOTOR_MAGIC  'h'  
#define POWER_DATA  _IOW(MOTOR_MAGIC,  1,int)






struct powerdata
{
	unsigned int pdata;
	unsigned int rev[10];
};



#endif