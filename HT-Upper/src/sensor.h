#include <BMI160Gen.h>
#include <Wire.h>
#include <QMC5883LCompass.h>

//BMI160
#define SDA_PIN_0 12
#define SCL_PIN_0 13
//QMC5883L
#define SDA_PIN_1 14
#define SCL_PIN_1 15

#define I2C_Frequency 400000

extern QMC5883LCompass compass;

#define LOW_PASS_FILTER_ALPHA 0.0001 // 低通滤波器系数
#define COMPLEMENTARY_FILTER_ALPHA 0.988 // 互补滤波器衰减系数

extern unsigned long samplingInterval; // 采样周期，单位为毫秒

extern float accX, accY, accZ;
extern float gyroX, gyroY, gyroZ;
extern float filteredGyroX, filteredGyroY, filteredGyroZ;
extern float pitch, roll;
extern int yaw, yawOffset;
extern float pitchOffset;
extern float rollOffset;

extern float absolutePitch, absoluteRoll;
extern float avgAccX, avgAccY, avgAccZ;
extern int count;

extern int Rawangle;

extern float X_OFFSET, Y_OFFSET, Z_OFFSET;
extern float X_SCALE ,Y_SCALE ,Z_SCALE;

extern float TX_battery_voltage;

float convertRawAcc(int aRaw);
float convertRawGyro(int gRaw);
void readSensorData();
float lowPassFilter(float currentMeasurement, float previousEstimate, float alpha);
void Battery_voltage();
void Compass_Calibration();
void do_Zeroing();