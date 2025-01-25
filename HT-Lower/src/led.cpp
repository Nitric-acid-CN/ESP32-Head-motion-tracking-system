#include "led.h"
#include "sensor.h"
#include "ppm.h"
#include "otg.h"
#include "config.h"
#include "wireless.h"

CRGB leds[NUM_LEDS];

unsigned long previousMillis = 0;
int currentPhase = 0; // 当前阶段：0 表示初始化，1 表示闪烁，2 表示延迟
int currentCount = 0;

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

void led() {
  if (millis() - lastReceived >= timeout && isMacSet() && isWifiWorking) {
    isConnected = false;
    debug(3, CRGB::Red, 100, 1000);
  }

  if (led_signal && isConnected && !(voltageToCharge(TX_battery_voltage) <= bat_warn_value) && !(voltageToCharge(RX_battery_voltage) <= bat_warn_value)) {
  int rssiPercent = esp_now_rssi();
    // 将延迟映射为红色和绿色之间的渐变
    uint8_t red = map(rssiPercent, 0, 100, 0, 255);   // 延迟为100时红色最大，为0时红色为0
    uint8_t green = map(rssiPercent, 0, 100, 255, 0); // 延迟为100时绿色为0，为0时绿色最大
    uint8_t blue = 0;  // 蓝色保持为0

    // 应用计算后的颜色到LEDs上
    for(int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(red, green, blue);
    }
    FastLED.show();
  }

  if (battery_warning) {
    if (voltageToCharge(RX_battery_voltage) <= bat_warn_value) debug(2, CRGB::Yellow, 500, 1000);
    if (voltageToCharge(TX_battery_voltage) <= bat_warn_value && isConnected) debug(2, CRGB::Yellow, 500, 1000);
  }
}