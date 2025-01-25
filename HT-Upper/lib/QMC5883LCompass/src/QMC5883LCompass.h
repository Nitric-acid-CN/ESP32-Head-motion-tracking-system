#ifndef QMC5883L_Compass
#define QMC5883L_Compass

#include "Arduino.h"
#include "Wire.h"

// QMC5883L寄存器配置宏定义
/*模式寄存器 (MODE) 定义*/

/*关闭模式*/
#define Standby_Mode 0x00
/*连续模式*/
#define Continuous_Mode 0x01

/*输出数据速率 (ODR) 定义*/
#define ODR_10Hz 0x00
#define ODR_50Hz 0x04
#define ODR_100Hz 0x08
#define ODR_200Hz 0x0C

/*比例 (RNG) (满量程) 定义*/
#define Full_Scale_2G 0x00
#define Full_Scale_8G 0x10

/*采样率 (OSR) 定义*/
#define OSR_512 0x00
#define OSR_256 0x40
#define OSR_128 0x80
#define OSR_64 0xC0

class QMC5883LCompass{
	
  public:
    QMC5883LCompass();
	void init();
    void setADDR(byte b);
    void setMode(byte mode, byte odr, byte rng, byte osr);
	void setMagneticDeclination(int degrees, uint8_t minutes);
	void setSmoothing(byte steps, bool adv);
	void calibrate();
	void setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max);
	void setCalibrationOffsets(float x_offset, float y_offset, float z_offset);
	void setCalibrationScales(float x_scale, float y_scale, float z_scale);
    float getCalibrationOffset(uint8_t index);
	float getCalibrationScale(uint8_t index);
	void clearCalibration();
	void setReset();
    void read();
	int getX();
	int getY();
	int getZ();
	int getAzimuth();
	int getAzimuthABS(float Pit, float Rol);
	byte getBearing(int azimuth);
	void getDirection(char* myArray, int azimuth);

  private:
    void _writeReg(byte reg,byte val);
	int _get(int index);
	float _magneticDeclinationDegrees = 0;
	bool _smoothUse = false;
	byte _smoothSteps = 5;
	bool _smoothAdvanced = false;
    byte _ADDR = 0x0D;
	int _vRaw[3] = {0,0,0};
	int _vHistory[10][3];
	int _vScan = 0;
	long _vTotals[3] = {0,0,0};
	int _vSmooth[3] = {0,0,0};
	void _smoothing();
	float _offset[3] = {0.,0.,0.};
	float _scale[3] = {1.,1.,1.};
	int _vCalibrated[3];
	void _applyCalibration();
	const char _bearings[16][3] =  {
		{' ', ' ', 'N'},
		{'N', 'N', 'E'},
		{' ', 'N', 'E'},
		{'E', 'N', 'E'},
		{' ', ' ', 'E'},
		{'E', 'S', 'E'},
		{' ', 'S', 'E'},
		{'S', 'S', 'E'},
		{' ', ' ', 'S'},
		{'S', 'S', 'W'},
		{' ', 'S', 'W'},
		{'W', 'S', 'W'},
		{' ', ' ', 'W'},
		{'W', 'N', 'W'},
		{' ', 'N', 'W'},
		{'N', 'N', 'W'},
	};
	
	
	
};

#endif
