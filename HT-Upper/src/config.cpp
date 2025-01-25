#include "sensor.h"
#include "config.h"
#include "wireless.h"

Preferences preferences;

int past_channel;

void load_config() {
    preferences.begin("config", true);

    X_OFFSET = preferences.getFloat("X_OFFSET", 0); 
    Y_OFFSET = preferences.getFloat("Y_OFFSET", 0);
    Z_OFFSET = preferences.getFloat("Z_OFFSET", 0);
    X_SCALE = preferences.getFloat("X_SCALE", 1); 
    Y_SCALE = preferences.getFloat("Y_SCALE", 1);
    Z_SCALE = preferences.getFloat("Z_SCALE", 1);

    // 逐个字节读取MAC地址
    for (int i = 0; i < 6; i++) {
        String macKey = "mac_" + String(i);
        receiverAddress[i] = preferences.getUInt(macKey.c_str(), 0x00);
    }

    preferences.end();

    // 应用读取的校准值
    compass.setCalibrationOffsets(X_OFFSET, Y_OFFSET, Z_OFFSET);
    compass.setCalibrationScales(X_SCALE, Y_SCALE, Z_SCALE);
}

void save_config() {
    preferences.begin("config", false);

    preferences.putFloat("X_OFFSET", X_OFFSET);
    preferences.putFloat("Y_OFFSET", Y_OFFSET);
    preferences.putFloat("Z_OFFSET", Z_OFFSET);
    preferences.putFloat("X_SCALE", X_SCALE);
    preferences.putFloat("Y_SCALE", Y_SCALE);
    preferences.putFloat("Z_SCALE", Z_SCALE);

    // 逐个字节存储MAC地址
    for (int i = 0; i < 6; i++) {
        String macKey = "mac_" + String(i);
        preferences.putUInt(macKey.c_str(), receiverAddress[i]);
    }

    preferences.end();
}

void unpair() {
    if (!Data_read.Unpairing) return;

    Data_sent.Unpairing = true;

    SentData(0);

    delay(1000);

    for (int i = 0; i < 6; i++) {
        receiverAddress[i] = 0;
    }

    save_config();

    delay(500);

    ESP.restart();
}

void channel_change() {
    if (channel != past_channel) {
        past_channel = channel;
        save_config();
    }
}