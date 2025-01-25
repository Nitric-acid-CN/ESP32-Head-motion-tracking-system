#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <WifiEspNow.h>

extern uint8_t receiverAddress[6]; // 接收方的MAC地址
extern uint8_t StaMac[6]; //本机MAC地址

extern unsigned long lastReceived; // 记录最后一次接收数据的时间
extern unsigned long millis1;
extern const int timeout; // 设置超时时间（毫秒）

extern bool isConnected, isWifiWorking, isMacgetting;
extern float pitch, roll;
extern int yaw;
extern float TX_battery_voltage,RX_battery_voltage;
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

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void Esp_now_setup();
bool isMacSet();
void getMac();
void SentData(int us /*Delay in us*/);
int esp_now_rssi();
void Esp_now_off();
void SentData(int us /*Delay in us*/);