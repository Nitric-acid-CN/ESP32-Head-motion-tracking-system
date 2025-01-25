#include "wireless.h"
#include "led.h"
#include "ui_main.h"
#include "config.h"

uint8_t receiverAddress[6]; // 接收方的MAC地址
uint8_t StaMac[6]; //本机MAC地址

const char *ssid = "Head tracker";
const char *password = "31415926";

esp_now_peer_info_t peerInfo;

unsigned long lastReceived = 0; // 记录最后一次接收数据的时间
unsigned long millis1;
const int timeout = 100; // 设置超时时间（毫秒）

bool isConnected = false, isWifiWorking = false, isMacgetting = false;
float pitch, roll;
int yaw;
float TX_battery_voltage, RX_battery_voltage;
int channel = 1;

Data Data_read;   //接收的数据
Data Data_sent;   //发送的数据

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&Data_read, incomingData, sizeof(Data_read));
  // 在这里处理接收到的数据
  pitch = Data_read.pitch;
  roll = Data_read.roll;
  yaw = Data_read.yaw;
  TX_battery_voltage = Data_read.battery_voltage;

  lastReceived = millis(); // 更新接收时间
  isConnected = true;
}

void Esp_now_setup() {
  if (!isMacSet()) return;
  if (isWifiWorking) return;

  while (!WiFi.mode(WIFI_MODE_STA)) {
    WiFi.mode(WIFI_MODE_STA);
  }

  //获取STA模式下的本机MAC地址
  while (esp_wifi_get_mac(WIFI_IF_STA, StaMac) != ESP_OK) {
    esp_wifi_get_mac(WIFI_IF_STA, StaMac); 
  }

  while (esp_wifi_set_mac(WIFI_IF_STA, StaMac) != ESP_OK) {
    esp_wifi_set_mac(WIFI_IF_STA, StaMac);
    debug(1, CRGB::White, 100, 1000);
  }

  if (esp_now_init() != ESP_OK) {
    return;
  }

  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = channel;  
  peerInfo.encrypt = false;
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  if (esp_now_register_recv_cb(OnDataRecv) == ESP_OK) {
    debug(1, CRGB::Green, 500, 1000);
  } else if (esp_now_register_recv_cb(OnDataRecv) == ESP_ERR_WIFI_NOT_INIT) {
    debug(1, CRGB::Red, 500, 200);
  } else if (esp_now_register_recv_cb(OnDataRecv) == ESP_ERR_ESPNOW_INTERNAL) {
    debug(2, CRGB::Red, 500, 200);
  }

  esp_wifi_set_max_tx_power(80);
  if (esp_wifi_set_max_tx_power(80) == ESP_OK) {
    debug(1, CRGB::Blue, 500, 1000);
  } else if (esp_wifi_set_max_tx_power(80) == ESP_ERR_WIFI_NOT_INIT) {
    debug(1, CRGB::Red, 500, 200);
  } else if (esp_wifi_set_max_tx_power(80) == ESP_ERR_WIFI_NOT_STARTED) {
    debug(2, CRGB::Red, 500, 200);
  } else {
    debug(3, CRGB::Red, 500, 200);
  }

  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);//省电模式
  if (esp_wifi_set_ps(WIFI_PS_MIN_MODEM) == ESP_OK) {
    debug(1, CRGB::Green, 500, 1000);
  } else if (esp_wifi_set_ps(WIFI_PS_MIN_MODEM) == ESP_ERR_WIFI_NOT_INIT) {
    debug(1, CRGB::Red, 500, 200);
  } else if (esp_wifi_set_ps(WIFI_PS_MIN_MODEM) == ESP_ERR_WIFI_NOT_STARTED) {
    debug(2, CRGB::Red, 500, 200);
  } else {
    debug(3, CRGB::Red, 500, 200);
  }

  isWifiWorking = true;
}

bool isMacSet() {
  for (int i = 0; i < 6; i++) {
    if (receiverAddress[i] != 0) {
      return true;
    }
  }
  return false;
}

void getMac() {
  if (!isMacgetting) return;
  if (isMacSet()) return;

  isMacgetting = false;

  // 还原 TemMac 的初始值
  uint8_t TemMac[6] = {0xC0, 0x4E, 0x30, 0x9E, 0x52, 0x80};
  
  while (!WiFi.mode(WIFI_MODE_STA)) {
    WiFi.mode(WIFI_MODE_STA);
  }

  //获取STA模式下的本机MAC地址
  while (esp_wifi_get_mac(WIFI_IF_STA, StaMac) != ESP_OK) {
    esp_wifi_get_mac(WIFI_IF_STA, StaMac); 
  }

  //将STA模式的MAC地址设置为临时MAC地址
  while (esp_wifi_set_mac(WIFI_IF_STA, TemMac) != ESP_OK) {
    esp_wifi_set_mac(WIFI_IF_STA, TemMac);
    debug(1, CRGB::Yellow, 100, 1000);
  }

  //打开热点
  while (!WiFi.softAP(ssid, password)) {
    WiFi.softAP(ssid, password);
  }

  esp_wifi_set_max_tx_power(80);
  esp_wifi_set_ps(WIFI_PS_NONE);

  //获取热点模式下Mac地址(即原来的STA模式本地MAC)
  while (esp_wifi_get_mac(WIFI_IF_AP, TemMac) != ESP_OK) {
    esp_wifi_get_mac(WIFI_IF_AP, TemMac); 
  }

  //将热点模式的MAC设置为STA模式的MAC
  while (esp_wifi_set_mac(WIFI_IF_AP, StaMac) != ESP_OK) {
    esp_wifi_set_mac(WIFI_IF_AP, StaMac);
    debug(2, CRGB::Yellow, 100, 1000);
  }

  wifi_sta_list_t wifi_sta_list;
  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
  esp_wifi_ap_get_sta_list(&wifi_sta_list);

  while (wifi_sta_list.num == 0) {
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    debug(1, CRGB::Red, 100, 100);
  }

  leds[0] = CRGB::Yellow; 
  FastLED.show(); 

  wifi_sta_info_t station = wifi_sta_list.sta[0];

  for (int j = 0; j < 6; j++) {
    receiverAddress[j] = station.mac[j];
  }

  while (wifi_sta_list.num != 0) {
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
  }

  while (esp_wifi_set_mac(WIFI_IF_AP, TemMac) != ESP_OK) {
    esp_wifi_set_mac(WIFI_IF_AP, TemMac);
    debug(2, CRGB::White, 100, 1000);
  }

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);

  save_config();
}

void SentData(int us /*Delay in us*/){
  if (!isWifiWorking && !isConnected) return;
  Data_sent.channel = channel;

  while (esp_now_send(receiverAddress, (uint8_t *) &Data_sent, sizeof(Data_sent)) != ESP_OK) {
    esp_now_send(receiverAddress, (uint8_t *) &Data_sent, sizeof(Data_sent));
  }

  delayMicroseconds(us);
}

int esp_now_rssi() {
  long signal_delay = millis() - lastReceived;
  return signal_delay;
}

void Esp_now_off() {
  esp_now_deinit();
  WiFi.mode(WIFI_OFF);
  isWifiWorking = false;
}