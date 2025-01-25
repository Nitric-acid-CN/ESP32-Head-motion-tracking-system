#include "wireless.h"
#include "sensor.h"
#include "config.h"

const char *ssid = "Head tracker";
const char *password = "31415926";

esp_now_peer_info_t peerInfo;

unsigned long lastReceived = 0; // 记录最后一次接收数据的时间
unsigned long millis1;
const int timeout = 100; // 设置超时时间（毫秒）

uint8_t receiverAddress[6]; // 接收方的MAC地址

int channel = 1;

Data Data_read;   //接收的数据
Data Data_sent;   //发送的数据

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&Data_read, incomingData, sizeof(Data_read));
  // 在这里处理接收到的数据
  channel = Data_read.channel;

  lastReceived = millis(); // 更新接收时间
  if (Data_read.Zeroing || Data_read.Calibrating) debug(2, CRGB::Blue, 200, 1000); else fancyColorChange();
}

void Esp_now_setup() {
  WiFi.mode(WIFI_MODE_STA);

  if (esp_now_init() != ESP_OK) {
    while (esp_now_init() != ESP_OK) {
      debug(1, CRGB::Red, 800, 3000);
    }
  }

  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = channel;  
  peerInfo.encrypt = false;
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    while (esp_now_add_peer(&peerInfo) != ESP_OK) {
      if (esp_now_add_peer(&peerInfo) == ESP_ERR_ESPNOW_NOT_INIT) {
        debug(1, CRGB::Yellow, 800, 3000);
      } else if (esp_now_add_peer(&peerInfo) == ESP_ERR_ESPNOW_ARG) {
        debug(2, CRGB::Yellow, 800, 3000);
      } else if (esp_now_add_peer(&peerInfo) == ESP_ERR_ESPNOW_NOT_FOUND) {
        debug(3, CRGB::Yellow, 800, 3000);
      }
    }
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
  while (!WiFi.begin(ssid, password)) {
    WiFi.begin(ssid, password);
    delay(100);
  }

  esp_wifi_set_ps(WIFI_PS_NONE);

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.status();
    debug(1, CRGB::Red, 100, 100);
  }

  leds[0] = CRGB::Yellow; 
  FastLED.show(); 
  
  wifi_ap_record_t info;
  if(!esp_wifi_sta_get_ap_info(&info)) {
    for (int i = 0; i < 6; i++) {
      receiverAddress[i] = info.bssid[i];
    }
  }

  uint8_t macAddr[6]; 
  esp_wifi_get_mac(WIFI_IF_STA, macAddr); 

  if (isMacSet()){
    leds[0] = CRGB::Green; 
    FastLED.show(); 
  }

  WiFi.disconnect();

  save_config();
}

void SentData(int us /*Delay in us*/){

  Data_sent.pitch = pitch;
  Data_sent.roll = roll;
  Data_sent.yaw = yaw;
  Data_sent.battery_voltage = TX_battery_voltage;

  while (esp_now_send(receiverAddress, (uint8_t *) &Data_sent, sizeof(Data_sent)) != ESP_OK) {
    esp_now_send(receiverAddress, (uint8_t *) &Data_sent, sizeof(Data_sent));
  }

  delayMicroseconds(us);
}