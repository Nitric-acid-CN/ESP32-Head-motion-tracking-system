#include "sensor.h"
#include "wireless.h"
#include "ui_main.h"
#include "config.h"
#include "otg.h"

unsigned long lastSampleTime = 0; // 上一次读取样本的时间
float alpha = 0.002; // 滤波系数，取值范围为 0 到 1，值越大响应速度越快

const int bmi160_interrupt_pin = 11;

unsigned long last_TouchTime = 0;
unsigned long lastTapTime = 0;  // 记录上次震动的时间
bool tapDetected = false;       // 记录是否检测到震动

void bmi160_intr(void){
  tapDetected = true;  // 记录中断发生
}

float lowPassFilter(float currentMeasurement, float previousEstimate, float alpha) {
  return alpha * currentMeasurement + (1 - alpha) * previousEstimate;
}

void Battery_voltage() {
  if (millis() - lastSampleTime >= 100) { 
    lastSampleTime = millis(); 

    float currentVoltage = analogRead(8) * (3.3 / 4095.0) * 8.04; // 读取当前电压

    RX_battery_voltage = lowPassFilter(currentVoltage,RX_battery_voltage,alpha);
  }
}

int voltageToCharge(float voltage) {
  if (voltage >= 4.20) return 100;
  if (voltage <= 3.60) return 0;
  return (voltage - 3.60) * (100.00 / (4.20 - 3.60));
}

void imu_init() {
  Wire1.begin(SDA_PIN_0, SCL_PIN_0, I2C_Frequency);
  BMI160.begin(BMI160GenClass::I2C_MODE, Wire1, 0x68, bmi160_interrupt_pin);
  BMI160.attachInterrupt(bmi160_intr);
  BMI160.setIntTapEnabled(true);
}

void double_touch_zeroing() {
  if (double_touch == false) return;
  static uint32_t firstTouchTime = 0;
  static bool firstTouchDetected = false;
  static bool waitForSecondTouch = false;

  // 检测到双击的第一个状态变化：isTouch -> !isTouch
  if (!firstTouchDetected && isTouch) {
    firstTouchDetected = true;  // 标记第一次触摸检测到
    waitForSecondTouch = true;  // 准备检测第二次触摸
    firstTouchTime = millis();  // 记录第一次触摸的时间
  }

  // 第二次检测，满足!isTouch -> isTouch的切换
  if (firstTouchDetected && !isTouch && waitForSecondTouch) {
    waitForSecondTouch = false;  // 等待第二次触摸
  }

  // 完整双击检测：isTouch -> !isTouch -> isTouch 且满足时间条件
  if (firstTouchDetected && isTouch && !waitForSecondTouch) {
    if (millis() - firstTouchTime < 500) {  // 时间间隔条件满足
      if (isConnected) {  // 确认设备已连接
        // 执行归零操作
        imu_cal = true;
        isEasy_1 = true;
        if (tud_mounted()) sendKey();  // 如果连接USB，则发送按键信号
        }
      }
      // 双击完成，重置状态
      firstTouchDetected = false;
  }

  // 如果时间过长（超过500ms），重置双击检测
  if (firstTouchDetected && millis() - firstTouchTime >= 500) {
    firstTouchDetected = false;
  }
}


void double_hit_zeroing() {
  if (double_hit) {  // 当 double_hit 为 true 时启用震动检测
    if (tapDetected) {
      tapDetected = false;  // 重置震动检测状态
      
      if (millis() - lastTapTime < 800) {  //检测到第二次震动
        if (isConnected) {  // 执行归零操作
          // 执行归零操作
          imu_cal = true;
          isEasy_1 = true;
          if (tud_mounted()) sendKey();
        } 
      }
      
      lastTapTime = millis();  // 更新最后一次震动时间
    }
  } else {
    if (tapDetected) {
      tapDetected = false;
    }
  }
}