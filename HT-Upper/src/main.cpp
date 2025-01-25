/*
  Head tracker v1.0 TX
*/

#include <Arduino.h>
#include <FastLED.h>
#include "wireless.h"
#include "sensor.h"
#include "config.h"

#define NUM_LEDS 1          
#define DATA_PIN 16       
#define BRIGHTNESS 60   // 设置LED的最大亮度
#define SPEED 10        // 设置颜色变换的速度

CRGB leds[NUM_LEDS];

unsigned long previousMillis = 0;
int currentPhase = 0; // 当前阶段：0 表示初始化，1 表示闪烁，2 表示延迟
int currentCount = 0;

void Task1(void *parameter);
void Task2(void *parameter);
void muti_core();

void nvs_clear() {
  preferences.begin("config", false);
  preferences.clear();
  preferences.end();
}

void debug(int count, CRGB color, int interval_time, int delay_time) {
  unsigned long currentMillis = millis();

  if (currentPhase == 0 && currentMillis - previousMillis >= delay_time) {
    // 初始化阶段，延迟时间结束后重置状态
    currentCount = 0;
    previousMillis = currentMillis;
    currentPhase = 1;
  }

  if (currentPhase == 1 && currentMillis - previousMillis >= interval_time) {
    // 闪烁阶段，按照间隔时间进行闪烁
    previousMillis = currentMillis;
    leds[0] = (leds[0] == CRGB::Black) ? color : CRGB::Black;
    FastLED.show();

    if (leds[0] == CRGB::Black) {
      currentCount++;
    }

    if (currentCount >= count) {
      // 达到闪烁次数，进入延迟阶段
      currentPhase = 2;
      previousMillis = currentMillis;
    }
  }

  if (currentPhase == 2 && currentMillis - previousMillis >= delay_time) {
    // 延迟阶段，延迟时间结束后重置状态
    currentPhase = 0;
  }
}

void fancyColorChange() {
  // 通过使用sin函数和时间作为变量，其中每种颜色的频率有所不同，来创建颜色变化效果
  uint8_t red = sin8(millis() * SPEED / 255);
  uint8_t green = sin8(millis() * SPEED / 255 + 85);
  uint8_t blue = sin8(millis() * SPEED / 255 + 170);

  // 应用计算后的颜色到LEDs上
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(red, green, blue);
  }
  FastLED.show();
}

void muti_core() {
  // 创建第一个任务，分配给核心 0
  xTaskCreatePinnedToCore(
    Task1,          // 任务函数
    "Task1",        // 任务名称
    10000,          // 堆栈大小
    NULL,           // 传递给任务的参数
    1,              // 任务优先级
    NULL,           // 任务句柄
    0               // 分配给的核心（0 或 1）
  ); 

  // 创建第二个任务，分配给核心 1
  xTaskCreatePinnedToCore(
    Task2,
    "Task2",
    10000,
    NULL,
    1,
    NULL,
    1
  );
}

void setup() {
  Wire.begin(SDA_PIN_0, SCL_PIN_0, I2C_Frequency);    //BMI160
  Wire1.begin(SDA_PIN_1, SCL_PIN_1, I2C_Frequency);   //QMC5883L

  BMI160.begin(BMI160GenClass::I2C_MODE);

  //compass.setMode(Continuous_Mode, ODR_200Hz, Full_Scale_2G, OSR_64);
  compass.init();
  compass.setSmoothing(10, true);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  leds[NUM_LEDS] = CRGB::Black;
  FastLED.show();

  //nvs_clear();

  load_config();

  Data_read.Zeroing = true;

  do_Zeroing();

  Data_read.Zeroing = false;

  TX_battery_voltage = analogRead(8) * (3.3 / 4095.0) * 8.0;

  millis1 = millis();

  past_channel = channel;

  muti_core();
}

void Task1(void *parameter) {
  while (true) {
    readSensorData();
    do_Zeroing();
    Compass_Calibration();
    unpair();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void Task2(void *parameter) {
  if (!isMacSet()) getMac();
  Esp_now_setup();
  while (true) {
    if (millis() - lastReceived >= timeout) {
      debug(3, CRGB::Red, 100, 1000);
    }

    Battery_voltage();

    channel_change();

    SentData(0);
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void loop() {}
