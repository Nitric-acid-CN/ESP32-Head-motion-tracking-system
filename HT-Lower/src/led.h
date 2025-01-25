#include <FastLED.h>

#define NUM_LEDS 1
#define DATA_PIN 16        
#define BRIGHTNESS 60   // 设置LED的最大亮度
#define SPEED 10        // 设置颜色变换的速度

extern CRGB leds[];

extern unsigned long previousMillis;
extern int currentPhase; // 当前阶段：0 表示初始化，1 表示闪烁，2 表示延迟
extern int currentCount;

extern int bat_warn_value;

void fancyColorChange();
void debug(int count, CRGB color, int interval_time, int delay_time);
void led();