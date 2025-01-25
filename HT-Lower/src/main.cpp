/*
  Head tracker v1.0 RX
*/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FastLED.h>
#include "wireless.h"
#include "ui_main.h"
#include "led.h"
#include "sensor.h"
#include "ppm.h"
#include "otg.h"
#include "config.h"

bool isSleep = false;

void Task1(void *parameter);
void Task2(void *parameter);
void muti_core();

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

void Scr_sleep() {
  if (millis() - lastTouchTime >= scr_time && !isSleep) {
    isSleep = true;
    analogWrite(TFT_BL, 255);
    if (isWifiWorking && !isConnected) Esp_now_off();
  }

  if (isSleep) {
    readTouchEvent(&touchData);
    isTouch = !touchData.isNull;
  }

  if (isTouch && isSleep) {
    isSleep = false;
    analogWrite(TFT_BL, (100 - brightness) / 100.0 * 255);
    if (!isWifiWorking && isMacSet()) Esp_now_setup();
  }
}

void nvs_clear() {
  preferences.begin("config", false);
  preferences.clear();
  preferences.end();
}

void setup() {
  Ui_setup();

  //nvs_clear();

  load_config();

  init_all();

  imu_init();

  USB.begin();
  Keyboard.begin();
  Mouse.begin();

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  leds[NUM_LEDS] = CRGB::Black;
  FastLED.show();

  RX_battery_voltage = analogRead(8) * (3.3 / 4095.0) * 8.0;

  analogWrite(TFT_BL, (100 - brightness) / 100.0 * 255); //屏幕亮度

  pinMode(ppmPin, OUTPUT);

  millis1 = millis();
  previousMillis = millis(); // 初始化计时

  muti_core();
}

void Task1(void *parameter) {
  Esp_now_setup();

  while (true) {
    SentData(0);

    PPM();

    processMouseMovement();

    Battery_voltage();

    getMac();

    Esp_now_setup();
    
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void Task2(void *parameter) {
  while (true) {
    Scr_sleep();

    imu_calibration_handler(isEasy_1);
    compass_calibration_handler();

    if (lv_scr_act() == ui_Screen1 && !isSleep) {
      UI_SR1();
      double_touch_zeroing();
    }

    if (lv_scr_act() == ui_Screen1) double_hit_zeroing();

    if (lv_scr_act() == ui_Screen4 && !isSleep) UI_SR4();

    if (lv_scr_act() == ui_Screen7 && !isSleep) UI_SR7();

    if (lv_scr_act() == ui_Screen8 && !isSleep) UI_SR8();

    if (!isSleep) lv_timer_handler();

    led();

    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void loop() {}
