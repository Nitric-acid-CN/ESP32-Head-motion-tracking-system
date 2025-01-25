#include <Preferences.h>
#include "ui_main.h"

extern Preferences preferences;

//设置变量
extern int brightness, scr_time, bat_warn_vaule;
extern int m_x, m_y, m_z, mouse_sens;
extern int yaw_channel, pitch_channel, roll_channel, yaw_max, pitch_max, roll_max, yaw_offset, pitch_offset, roll_offset;
extern bool led_signal, battery_warning, double_touch, double_hit;
extern bool yaw_channel_turn, pitch_channel_turn, roll_channel_turn;
extern bool imu_cal, compass_cal;

extern bool isEasy_1;

void save_config();
void load_config();
void init_all();

void set_brightness(lv_event_t * e);
void set_scr_time(lv_event_t * e);
void set_led_signal(lv_event_t * e);
void set_battery_warning(lv_event_t * e);
void set_bat_warn_value(lv_event_t * e);
void set_channel(lv_event_t * e);
void set_double_touch(lv_event_t * e);
void set_double_hit(lv_event_t * e);
void set_m_x(lv_event_t * e);
void set_m_y(lv_event_t * e);
void set_m_z(lv_event_t * e);
void mouse_sens_m(lv_event_t * e);
void mouse_sens_p(lv_event_t * e);
void set_yaw_channel(lv_event_t * e);
void set_pitch_channel(lv_event_t * e);
void set_roll_channel(lv_event_t * e);
void set_yaw_max(lv_event_t * e);
void set_pitch_max(lv_event_t * e);
void set_roll_max(lv_event_t * e);
void set_yaw_offset_m(lv_event_t * e);
void set_yaw_offset_p(lv_event_t * e);
void set_pitch_offset_m(lv_event_t * e);
void set_pitch_offset_p(lv_event_t * e);
void set_roll_offset_m(lv_event_t * e);
void set_roll_offset_p(lv_event_t * e);
void pairing(lv_event_t * e);
void unpair(lv_event_t * e);
void imu_calibration(lv_event_t * e);
void compass_calibration(lv_event_t * e);

void imu_calibration_handler(bool isEasy);
void compass_calibration_handler();