//#include<time.h>
//#include<sys/time.h>
//#include<unistd.h>
#include<motor_drv.h>
//#include<stdint.h>



//unsigned int   MusicStartFlag;
//unsigned int  MusicStopFlag;

//#define C_ROBOTDANCE_START   'O'
//#define C_ROBOTDANCE_STOP 'P'

 void RobotDance_Stop(void);
 void RobotDanceAction_I(void);
 void RobotDanceAction_II(void);
 void RobotDanceAction_III(void);
 void RobotDanceAction_IV(void);
 
 void RobotDanceAction_11_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_12_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_13_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_14_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_15_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_21_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_22_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_23_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_24_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_25_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_31_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_32_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_33_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_34_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_35_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_41_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_42_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_43_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_44_P(unsigned int fast,unsigned int slow);
 void RobotDanceAction_45_P(unsigned int fast,unsigned int slow);
 
 int MOTOR_FORWARD_P(unsigned int fast,unsigned int slow) ;
 int MOTOR_BACK_P(unsigned int fast,unsigned int slow);		   
 int MOTOR_LEFT_P(unsigned int fast,unsigned int slow);		   
 int MOTOR_RIGHT_P(unsigned int fast,unsigned int slow);


 int MOTOR_HEAD_FORWARD_P(unsigned int fast,unsigned int slow);  
 int MOTOR_HEAD_BACK_P(unsigned int fast,unsigned int slow);			
 int MOTOR_HEAD_LEFT_P(unsigned int fast,unsigned int slow);   			
 int MOTOR_HEAD_RIGHT_P(unsigned int fast,unsigned int slow);   		

 void MOTOR_STOP_P(void);
 void MOTOR_V_STOP_P(void); 		   
 void MOTOR_H_STOP_P(void); 
 
 void Motor_Test(void);
 void RobotPreAction(void);
 void RobotPreAction_P(unsigned int fast,unsigned int slow);

