#include <linux/ioctl.h>



#define    motor_name     "motor"



#define MOTOR_MAGIC  'm'  
#define ROBOT_FORWORD   _IOW(MOTOR_MAGIC,  1,int)
#define ROBOT_BACK   _IOW(MOTOR_MAGIC,  2,int)
#define ROBOT_LEFT   _IOW(MOTOR_MAGIC,  3,int)
#define ROBOT_RIGHT   _IOW(MOTOR_MAGIC,  4,int)
#define ROBOT_STOP   _IOW(MOTOR_MAGIC,  10,int)


#define ROBOT_HEAD_UP   _IOW(MOTOR_MAGIC,  5,int)
#define ROBOT_HEAD_DOWN   _IOW(MOTOR_MAGIC,  6,int)
#define ROBOT_HEAD_LEFT   _IOW(MOTOR_MAGIC,  7,int)
#define ROBOT_HEAD_RIGHT   _IOW(MOTOR_MAGIC,  8,int)
#define ROBOT_HEAD_STOP   _IOW(MOTOR_MAGIC,  20,int)

#define C_ROBOT_FORWORD  'A'
#define C_ROBOT_BACK   'B'
#define C_ROBOT_LEFT   'C'
#define C_ROBOT_RIGHT   'D'
#define C_ROBOT_STOP   'E'

#define C_ROBOT_HEAD_UP  'I'
#define C_ROBOT_HEAD_DOWN  'J'
#define C_ROBOT_HEAD_LEFT  'K'
#define C_ROBOT_HEAD_RIGHT   'L'
#define C_ROBOT_H_STOP   'M'
#define C_ROBOT_V_STOP   'S'

#define C_ROBOT_STOP_DANCE   'N'
#define C_ROBOT_START_DANCE   'H'


#define GPIO44_PIN   (GPIO44|0x80000000)
#define GPIO10_PIN   (GPIO10|0x80000000)

#define GPIO95_PIN   (GPIO95|0x80000000)
#define GPIO96_PIN   (GPIO96|0x80000000)

#define GPIO60_PIN   (GPIO60|0x80000000)
#define GPIO21_PIN   (GPIO21|0x80000000)

#define GPIO99_PIN   (GPIO99|0x80000000)
#define GPIO100_PIN   (GPIO100|0x80000000)

#define GPIO101_PIN   (GPIO101|0x80000000)
#define GPIO102_PIN   (GPIO102|0x80000000)

#define GPIO42_PIN   (GPIO42|0x80000000)
#define GPIO43_PIN   (GPIO43|0x80000000)
#define GPIO44_PIN   (GPIO44|0x80000000)

#define GPIO43_PIN (GPIO43|0x80000000)

//void motor_h_stop(void);

