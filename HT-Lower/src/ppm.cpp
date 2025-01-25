#include "ppm.h"
#include "config.h"
#include "wireless.h"

// PPM输出管脚
const int ppmPin = 9; 

// PPM配置参数
#define channelNumber 8      // PPM通道数
int channelValues[channelNumber] = {}; // 各通道信号长度（微秒）
int frameLength = 10000;     // PPM帧时长（微秒）

int mapYawToSignedValue(int yaw_val, int yaw_mode) {
    switch (yaw_mode) {
        case 360: // 360度模式
            if (yaw_val > 180) {
                yaw_val = yaw_val - 360; // 将范围转为 [-180, 180]
            }
            break;

        case 270: // 270度模式
            if (yaw_val > 135) {
                yaw_val = yaw_val - 270;  // 保持负数在 [-135, 0]
            }
            break;

        case 180: // 180度模式
            if (yaw_val > 90) {
                yaw_val = yaw_val - 180;  // 范围变为 [-180, 180]
            }
            break;

        default:
            Serial.println("Invalid yaw mode"); // 错误处理
            break;
    }

    return yaw_val; // 返回转为带正负的yaw值
}

void PPM() {
    if (tud_mounted()) return;
    if (isConnected) {
        int yaw_cov = mapYawToSignedValue(yaw, yaw_max) + yaw_offset;

        // Yaw的映射逻辑
        if (yaw_channel_turn) {
            // 反向映射
            channelValues[yaw_channel] = map(yaw_cov, -yaw_max / 2, yaw_max / 2, 2012, 988);
        } else {
            // 正向映射
            channelValues[yaw_channel] = map(yaw_cov, -yaw_max / 2, yaw_max / 2, 988, 2012);
        }

        // Pitch的映射逻辑（中点为0，正负pitch_max/2）
        if (pitch_channel_turn) {
        // 反向映射
            channelValues[pitch_channel] = map(pitch + pitch_offset, -pitch_max / 2, pitch_max / 2, 2012, 988);
        } else {
            // 正向映射
            channelValues[pitch_channel] = map(pitch + pitch_offset, -pitch_max / 2, pitch_max / 2, 988, 2012);
        }

        // Roll的映射逻辑（中点为0，正负roll_max/2）
        if (roll_channel_turn) {
            // 反向映射
            channelValues[roll_channel] = map(roll + roll_offset, -roll_max / 2, roll_max / 2, 2012, 988);
        } else {
            // 正向映射
            channelValues[roll_channel] = map(roll + roll_offset, -roll_max / 2, roll_max / 2, 988, 2012);
        }
    } else {
        channelValues[yaw_channel] = 1500;
        channelValues[pitch_channel] = 1500;
        channelValues[roll_channel] = 1500;
    }
    
    unsigned long totalPulseTime = 0;

    digitalWrite(ppmPin, HIGH);
    digitalWrite(ppmPin, LOW);

    for (int i = 0; i < channelNumber; i++) {
        digitalWrite(ppmPin, HIGH);
        digitalWrite(ppmPin, LOW);
        delayMicroseconds(channelValues[i]);
        totalPulseTime += channelValues[i];
    }

    digitalWrite(ppmPin, HIGH);
    digitalWrite(ppmPin, LOW);

    // 最后一个间隔以保持整个帧时长
    delayMicroseconds(frameLength - totalPulseTime);
}