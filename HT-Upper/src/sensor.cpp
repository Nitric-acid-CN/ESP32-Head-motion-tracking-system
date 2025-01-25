#include "sensor.h"
#include "config.h"
#include "wireless.h"

QMC5883LCompass compass;

float alpha = 0.002; // 滤波系数，取值范围为 0 到 1，值越大响应速度越快 (Battery)

unsigned long samplingInterval = 10; // 采样周期，单位为毫秒

float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float filteredGyroX = 0, filteredGyroY = 0, filteredGyroZ = 0;
float pitch = 0, roll = 0;
int yaw = 0, yawOffset = 0;
float pitchOffset = 0;
float rollOffset = 0;
int angle;

float absolutePitch, absoluteRoll;

float avgAccX = 0, avgAccY = 0, avgAccZ = 0;
int count = 0;

int Rawangle;

float TX_battery_voltage;

float X_OFFSET, Y_OFFSET, Z_OFFSET;
float X_SCALE ,Y_SCALE ,Z_SCALE;

float convertRawAcc(int aRaw) {
  // since we are using ±2g range
  // -2g maps to a raw value of -32768
  // +2g maps to a raw value of 32767
  float a = (aRaw * 2.0) / 32768.0;
  return a;
}

float convertRawGyro(int gRaw) {
  // since we are using ±250 degrees/second range
  // -250 maps to a raw value of -32768
  // +250 maps to a raw value of 32767
  float g = (gRaw * 250.0) / 32768.0;
  return g;
}

float lowPassFilter(float currentMeasurement, float previousEstimate, float alpha) {
    return alpha * currentMeasurement + (1 - alpha) * previousEstimate;
}

void readSensorData() {
    int accRaw[3], gyroRaw[3];
    BMI160.readAccelerometer(accRaw[0], accRaw[1], accRaw[2]);
    BMI160.readGyro(gyroRaw[0], gyroRaw[1], gyroRaw[2]);

    // 将原始传感器数据转换为物理值，并考虑偏移量
    accX = convertRawAcc(accRaw[0]) - avgAccX;
    accY = convertRawAcc(accRaw[1]) - avgAccY;
    accZ = convertRawAcc(accRaw[2]);

    // 对陀螺仪数据应用低通滤波器
    filteredGyroX = LOW_PASS_FILTER_ALPHA * filteredGyroX + (1 - LOW_PASS_FILTER_ALPHA) * convertRawGyro(gyroRaw[0]);
    filteredGyroY = LOW_PASS_FILTER_ALPHA * filteredGyroY + (1 - LOW_PASS_FILTER_ALPHA) * convertRawGyro(gyroRaw[1]);
    filteredGyroZ = LOW_PASS_FILTER_ALPHA * filteredGyroZ + (1 - LOW_PASS_FILTER_ALPHA) * convertRawGyro(gyroRaw[2]);

    // 使用互补滤波器计算俯仰角和横滚角
    float accPitch = atan2(accY, sqrt(accX * accX + accZ * accZ)) * 180.0 / PI;
    float accRoll = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI;

    // 计算实际的俯仰角、横滚角
    pitch = (COMPLEMENTARY_FILTER_ALPHA * (pitch + filteredGyroX * 0.005) + (1 - COMPLEMENTARY_FILTER_ALPHA) * accPitch);
    roll = (COMPLEMENTARY_FILTER_ALPHA * (roll + filteredGyroY * 0.005) + (1 - COMPLEMENTARY_FILTER_ALPHA) * accRoll);

    // 计算绝对的俯仰角和横滚角
    absolutePitch = pitch + pitchOffset;
    absoluteRoll = roll + rollOffset;

    // 获取绝对的航向角（范围为±180度）
    compass.read();
    int current_angle = compass.getAzimuthABS(absolutePitch, absoluteRoll);
    angle = lowPassFilter(current_angle, angle, 0.06); //低通滤波器

    // 将罗盘数据转换为0到360度的范围，并减去零点偏移量
    yaw = (angle - yawOffset) % 360;

    if (yaw < 0) yaw += 360;
}

void Battery_voltage() {
    float currentVoltage = analogRead(8) * (3.3 / 4095.0) * 8.04; // 读取当前电压
    TX_battery_voltage = lowPassFilter(currentVoltage, TX_battery_voltage, alpha);
}

void Compass_Calibration() {
    Data_sent.Calibrating = false;

    if (!Data_read.Calibrating) return;

    // 执行校准流程
    compass.calibrate();

    // 获取当前校准偏移量并保存至变量
    X_OFFSET = compass.getCalibrationOffset(0);
    Y_OFFSET = compass.getCalibrationOffset(1);
    Z_OFFSET = compass.getCalibrationOffset(2);

    // 应用校准偏移量
    compass.setCalibrationOffsets(X_OFFSET, Y_OFFSET, Z_OFFSET);

    // 获取当前校准比例并保存至变量
    X_SCALE = compass.getCalibrationScale(0);
    Y_SCALE = compass.getCalibrationScale(1);
    Z_SCALE = compass.getCalibrationScale(2);

    // 取得新的校准值，并保存到NVS中
    save_config();

    // 应用校准比例
    compass.setCalibrationScales(X_SCALE, Y_SCALE, Z_SCALE);

    Data_sent.Calibrating = true;
}

void do_Zeroing() {
    Data_sent.Zeroing = false;

    if (!Data_read.Zeroing) return;

    // 重置计数器和平均值
    count = 0;
    avgAccX = 0;
    avgAccY = 0;
    avgAccZ = 0;
    Rawangle = 0;

    unsigned long startTime = millis();
    // 记录100ms内数据
    while (millis() - startTime < 100) {  
        int accRaw[3];
        BMI160.readAccelerometer(accRaw[0], accRaw[1], accRaw[2]);
        avgAccX += convertRawAcc(accRaw[0]);
        avgAccY += convertRawAcc(accRaw[1]);
        avgAccZ += convertRawAcc(accRaw[2]);
        count++;
        delay(samplingInterval);
    }

    // 计算平均值
    avgAccX /= count;
    avgAccY /= count;
    avgAccZ /= count;

    // 计算初始的pitch和roll偏移
    pitchOffset = atan2(avgAccY, sqrt(avgAccX * avgAccX + avgAccZ * avgAccZ)) * 180.0 / PI;
    rollOffset = atan2(-avgAccX, sqrt(avgAccY * avgAccY + avgAccZ * avgAccZ)) * 180.0 / PI;

    while (Rawangle == 0){
        // 获取绝对的航向角（范围为±180度）
        compass.read();
        Rawangle = compass.getAzimuthABS(absolutePitch, absoluteRoll);
        // 将罗盘数据转换为0到360度的范围，并减去零点偏移量
        yawOffset = Rawangle % 360;
    }

    Data_sent.Zeroing = true;
}