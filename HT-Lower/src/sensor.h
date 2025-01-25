#include <BMI160Gen.h>
#include <Wire.h>

//BMI160
#define SDA_PIN_0 12
#define SCL_PIN_0 13

#define I2C_Frequency 800000

extern bool tapDetected;

void Battery_voltage();
int voltageToCharge(float voltage);
void double_touch_zeroing();
void double_hit_zeroing();
void imu_init();
void bmi160_intr(void);