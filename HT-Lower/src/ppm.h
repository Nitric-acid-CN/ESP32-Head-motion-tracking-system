#include <Arduino.h>

// PPM输出管脚
extern const int ppmPin; 

// PPM配置参数
extern int channelValues[]; 
extern int frameLength;     

extern volatile unsigned long timerCounter;

int mapYawToSignedValue(int yaw_val, int yaw_mode);
void PPM();