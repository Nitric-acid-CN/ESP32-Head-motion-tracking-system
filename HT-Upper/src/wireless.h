#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <WifiEspNow.h>
#include <FastLED.h>

extern const char *ssid;
extern const char *password;

extern esp_now_peer_info_t peerInfo;

extern unsigned long lastReceived; // 记录最后一次接收数据的时间
extern unsigned long millis1;
extern const int timeout; // 设置超时时间（毫秒）

extern uint8_t receiverAddress[]; // 接收方的MAC地址

extern float battery_voltage;
extern int channel;

typedef struct {
    bool Calibrating;
    bool Unpairing;
    bool Zeroing;
    int channel;
    float pitch;
    float roll;
    int yaw;
    float battery_voltage;
} Data;

extern Data Data_read;   //接收的数据
extern Data Data_sent;   //发送的数据

extern CRGB leds[];

void fancyColorChange();
void debug(int count, CRGB color, int interval_time, int delay_time);

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void Esp_now_setup();
bool isMacSet();
void getMac();
void SentData(int us /*Delay in us*/);