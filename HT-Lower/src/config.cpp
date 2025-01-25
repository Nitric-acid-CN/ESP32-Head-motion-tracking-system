#include "config.h"
#include "wireless.h"
#include "ui_main.h"
#include "led.h"

Preferences preferences;

enum IMU_CalibrationState {
    IMU_IDLE,
    IMU_START,
    IMU_WAITING,
    IMU_DONE
};

IMU_CalibrationState imuCalState = IMU_IDLE;
long imuStartTime = 0;
bool imuFail = false;

enum Compass_CalibrationState {
    COMPASS_IDLE,
    COMPASS_START,
    COMPASS_WAITING,
    COMPASS_DONE
};

Compass_CalibrationState compassCalState = COMPASS_IDLE;
long compassStartTime = 0;
bool compassFail = false;

bool isEasy_1 = false;

//设置变量
int brightness = 50, scr_time = 60000 * 1, bat_warn_value = 20;
int m_x = 0, m_y = 1, m_z = 0, mouse_sens = 5;
int yaw_channel = 1, pitch_channel = 2, roll_channel = 3, yaw_max = 360, pitch_max = 90, roll_max = 60;
int yaw_offset = 0, pitch_offset = 0, roll_offset = 0;
bool led_signal = true, battery_warning = true, double_touch = true, double_hit = false;
bool yaw_channel_turn = false, pitch_channel_turn = false, roll_channel_turn = false;
bool imu_cal = false, compass_cal = false;

void save_config() {
    // 启动Preferences以保存配置
    preferences.begin("config", false);

    // 保存亮度设置
    preferences.putInt("brightness", brightness);

    // 保存屏幕时间设置
    preferences.putInt("scr_time", scr_time);

    // 保存LED信号设置
    preferences.putBool("led_signal", led_signal);

    // 保存电池警告设置
    preferences.putBool("battery_warning", battery_warning);

    // 保存电池警告值
    preferences.putInt("bat_warn_value", bat_warn_value);

    // 保存通道设置
    preferences.putInt("channel", channel);

    // 保存双击触摸设置
    preferences.putBool("double_touch", double_touch);

    // 保存双击击打设置
    preferences.putBool("double_hit", double_hit);

    // 保存鼠标轴设置
    preferences.putInt("m_x", m_x);
    preferences.putInt("m_y", m_y);
    preferences.putInt("m_z", m_z);

    // 保存鼠标灵敏度
    preferences.putInt("mouse_sens", mouse_sens);

    // 保存yaw、pitch、roll的通道设置
    preferences.putInt("yaw_channel", yaw_channel);
    preferences.putInt("pitch_channel", pitch_channel);
    preferences.putInt("roll_channel", roll_channel);

    // 保存yaw、pitch、roll的通道反向设置，默认为false
    yaw_channel_turn = preferences.putBool("yaw_channel_turn", false);
    pitch_channel_turn = preferences.putBool("pitch_channel_turn", false);
    roll_channel_turn = preferences.putBool("roll_channel_turn", false);

    // 保存yaw、pitch、roll的最大值
    preferences.putInt("yaw_max", yaw_max);
    preferences.putInt("pitch_max", pitch_max);
    preferences.putInt("roll_max", roll_max);

    // 保存yaw、pitch、roll的偏移量
    preferences.putInt("yaw_offset", yaw_offset);
    preferences.putInt("pitch_offset", pitch_offset);
    preferences.putInt("roll_offset", roll_offset);

    // 逐个字节存储MAC地址
    for (int i = 0; i < 6; i++) {
        String macKey = "mac_" + String(i);
        preferences.putUInt(macKey.c_str(), receiverAddress[i]);
    }

    // 结束Preferences存储
    preferences.end();
}

void load_config() {
    // 启动Preferences以加载配置
    preferences.begin("config", true);  // "true" 表示只读模式

    // 加载亮度设置，默认为100
    brightness = preferences.getInt("brightness", 50);

    // 加载屏幕时间设置，默认为1分钟（60000 ms）
    scr_time = preferences.getInt("scr_time", 60000 * 1);

    // 加载LED信号设置，默认为关闭（false）
    led_signal = preferences.getBool("led_signal", false);

    // 加载电池警告设置，默认为关闭（false）
    battery_warning = preferences.getBool("battery_warning", false);

    // 加载电池警告值，默认为20%
    bat_warn_value = preferences.getInt("bat_warn_value", 20);

    // 加载通道设置，默认为1
    channel = preferences.getInt("channel", 1);

    // 加载双击触摸设置，默认为关闭（false）
    double_touch = preferences.getBool("double_touch", false);

    // 加载双击击打设置，默认为关闭（false）
    double_hit = preferences.getBool("double_hit", false);

    // 加载鼠标轴设置
    m_x = preferences.getInt("m_x", 0);
    m_y = preferences.getInt("m_y", 1);
    m_z = preferences.getInt("m_z", 0);

    // 加载鼠标灵敏度，默认为5
    mouse_sens = preferences.getInt("mouse_sens", 5);

    // 加载yaw、pitch、roll的通道设置，默认为1
    yaw_channel = preferences.getInt("yaw_channel", 1);
    pitch_channel = preferences.getInt("pitch_channel", 2);
    roll_channel = preferences.getInt("roll_channel", 3);

    // 加载yaw、pitch、roll的通道反向设置，默认为false
    yaw_channel_turn = preferences.getBool("yaw_channel_turn", false);
    pitch_channel_turn = preferences.getBool("pitch_channel_turn", false);
    roll_channel_turn = preferences.getBool("roll_channel_turn", false);

    // 加载yaw、pitch、roll的最大值设置，默认为360/90/60
    yaw_max = preferences.getInt("yaw_max", 360);
    pitch_max = preferences.getInt("pitch_max", 90);
    roll_max = preferences.getInt("roll_max", 60);

    // 加载yaw、pitch、roll的偏移量，默认为0
    yaw_offset = preferences.getInt("yaw_offset", 0);
    pitch_offset = preferences.getInt("pitch_offset", 0);
    roll_offset = preferences.getInt("roll_offset", 0);

    // 逐个字节读取MAC地址
    for (int i = 0; i < 6; i++) {
        String macKey = "mac_" + String(i);
        receiverAddress[i] = preferences.getUInt(macKey.c_str(), 0x00);
    }

    // 结束Preferences读取
    preferences.end();
}


void set_brightness (lv_event_t * e) {
    brightness = lv_slider_get_value(ui_Slider1);
    analogWrite(TFT_BL, (100 - brightness) / 100.0 * 255);
    save_config();
}

void set_scr_time (lv_event_t * e) {
    // 获取选中的索引
    int selected = lv_dropdown_get_selected(ui_Dropdown1);

    // 根据选中的索引进行判断
    switch (selected) {
        case 0:
            // 选中 "1 min"
            scr_time = 60000 * 1;
            break;
        case 1:
            // 选中 "2 min"
            scr_time = 60000 * 2;  
            break;
        case 2:
            // 选中 "3 min"
            scr_time = 60000 * 3;
            break;
        case 3:
            // 选中 "4 min"
            scr_time = 60000 * 4;
            break;
        case 4:
            // 选中 "5 min"
            scr_time = 60000 * 5;
            break;
        case 5:
            // 选中 "10 min"
            scr_time = 60000 * 10;
            break;
        default:
            // 处理其他情况
            break;
    }
    save_config();
}

void set_led_signal(lv_event_t * e) {
    led_signal = lv_obj_has_state(ui_Switch1, LV_STATE_CHECKED);
    save_config();
}

void set_battery_warning (lv_event_t * e) {
    battery_warning = lv_obj_has_state(ui_Switch2, LV_STATE_CHECKED);
    save_config();
}

void set_bat_warn_value (lv_event_t * e) {
    // 获取选中的索引
    int selected = lv_dropdown_get_selected(ui_Dropdown2);

    // 根据选中的索引进行判断
    switch (selected) {
        case 0:
            // 选中 "50%"
            bat_warn_value = 50;
            break;
        case 1:
            // 选中 "40%"
            bat_warn_value = 40;  
            break;
        case 2:
            // 选中 "30%"
            bat_warn_value = 30;
            break;
        case 3:
            // 选中 "20%"
            bat_warn_value = 20;
            break;
        case 4:
            // 选中 "10%"
            bat_warn_value = 10;
            break;
        default:
            // 处理其他情况
            break;
    }
    save_config();
}

void set_channel(lv_event_t * e) {
    if (isConnected) {
        int selected = lv_dropdown_get_selected(ui_Dropdown12);

        channel = selected + 1;

        Data_sent.channel = channel;

        save_config();
    }
}

void set_double_touch(lv_event_t * e) {
    double_touch = lv_obj_has_state(ui_Switch3, LV_STATE_CHECKED);
    save_config();
}

void set_double_hit(lv_event_t * e) {
    double_hit = lv_obj_has_state(ui_Switch4, LV_STATE_CHECKED);
    save_config();
}

void set_m_x (lv_event_t * e) {
    // 获取选中的索引
    m_x = lv_dropdown_get_selected(ui_Dropdown3);
    save_config();
}

void set_m_y (lv_event_t * e) {
    // 获取选中的索引
    m_y = lv_dropdown_get_selected(ui_Dropdown4);
    save_config();
}

void set_m_z (lv_event_t * e) {
    // 获取选中的索引
    m_z = lv_dropdown_get_selected(ui_Dropdown5);
    save_config();
}

void mouse_sens_p(lv_event_t * e) {
    if (mouse_sens >= 10) mouse_sens = 10; else mouse_sens++;
    save_config();
}

void mouse_sens_m(lv_event_t * e) {
    if (mouse_sens <= 1) mouse_sens = 1; else mouse_sens--;
    save_config();
}

void set_yaw_channel (lv_event_t * e) {
    // 获取选中的索引
    yaw_channel = lv_dropdown_get_selected(ui_Dropdown6);

    if (yaw_channel > 7) {
        yaw_channel = yaw_channel - 7;
        yaw_channel_turn = true;
    }

    save_config();
}

void set_pitch_channel (lv_event_t * e) {
    // 获取选中的索引
    pitch_channel = lv_dropdown_get_selected(ui_Dropdown7);

    if (pitch_channel > 7) {
        pitch_channel = yaw_channel - 7;
        pitch_channel_turn = true;
    }

    save_config();
}

void set_roll_channel (lv_event_t * e) {
    // 获取选中的索引
    roll_channel = lv_dropdown_get_selected(ui_Dropdown8);

    if (roll_channel > 7) {
        roll_channel = yaw_channel - 7;
        roll_channel_turn = true;
    }

    save_config();
}

void set_yaw_max (lv_event_t * e) {
    // 获取选中的索引
    int selected = lv_dropdown_get_selected(ui_Dropdown9);

    // 根据选中的索引进行判断
    switch (selected) {
        case 0:
            // 选中 "180"
            yaw_max = 180;
            break;
        case 1:
            // 选中 "270"
            yaw_max = 270;  
            break;
        case 2:
            // 选中 "360"
            yaw_max = 360;
            break;
        default:
            // 处理其他情况
            break;
    }
    save_config();
}

void set_pitch_max (lv_event_t * e) {
    // 获取选中的索引
    int selected = lv_dropdown_get_selected(ui_Dropdown10);

    // 根据选中的索引进行判断
    switch (selected) {
        case 0:
            // 选中 "60"
            pitch_max = 60;
            break;
        case 1:
            // 选中 "90"
            pitch_max = 90;  
            break;
        default:
            // 处理其他情况
            break;
    }
    save_config();
}

void set_roll_max (lv_event_t * e) {
    // 获取选中的索引
    int selected = lv_dropdown_get_selected(ui_Dropdown11);

    // 根据选中的索引进行判断
    switch (selected) {
        case 0:
            // 选中 "60"
            roll_max = 60;
            break;
        case 1:
            // 选中 "90"
            roll_max = 90;  
            break;
        default:
            // 处理其他情况
            break;
    }
    save_config();
}

void set_yaw_offset_p(lv_event_t * e) {
    if (yaw_offset >= 20) yaw_offset = 20; else yaw_offset++;
    save_config();
}

void set_yaw_offset_m(lv_event_t * e) {
    if (yaw_offset <= -20) yaw_offset = -20; else yaw_offset--;
    save_config();
}

void set_pitch_offset_p(lv_event_t * e) {
    if (pitch_offset >= 20) pitch_offset = 20; else pitch_offset++;
    save_config();
}

void set_pitch_offset_m(lv_event_t * e) {
    if (pitch_offset <= -20) pitch_offset = -20; else pitch_offset--;
    save_config();
}

void set_roll_offset_p(lv_event_t * e) {
    if (roll_offset >= 20) roll_offset = 20; else roll_offset++;
    save_config();
}

void set_roll_offset_m(lv_event_t * e) {
    if (roll_offset <= -20) roll_offset = -20; else roll_offset--;
    save_config();
}

// 初始化亮度Slider
void init_brightness() {
    lv_slider_set_value(ui_Slider1, brightness, LV_ANIM_OFF);
    analogWrite(TFT_BL, (110 - brightness) / 100.0 * 255);
}

// 初始化屏幕时间Dropdown
void init_scr_time() {
    int selected;
    switch (scr_time) {
        case 60000 * 1:
            selected = 0;
            break;
        case 60000 * 2:
            selected = 1;
            break;
        case 60000 * 3:
            selected = 2;
            break;
        case 60000 * 4:
            selected = 3;
            break;
        case 60000 * 5:
            selected = 4;
            break;
        case 60000 * 10:
            selected = 5;
            break;
        default:
            selected = 0; // 默认选择1分钟
            break;
    }
    lv_dropdown_set_selected(ui_Dropdown1, selected);
}

// 初始化LED信号Switch
void init_led_signal() {
    if (led_signal) {
        lv_obj_add_state(ui_Switch1, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_Switch1, LV_STATE_CHECKED);
    }
}

// 初始化电池警告Switch
void init_battery_warning() {
    if (battery_warning) {
        lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_Switch2, LV_STATE_CHECKED);
    }
}

// 初始化电池警告值Dropdown
void init_bat_warn_value() {
    int selected;
    switch (bat_warn_value) {
        case 50:
            selected = 0;
            break;
        case 40:
            selected = 1;
            break;
        case 30:
            selected = 2;
            break;
        case 20:
            selected = 3;
            break;
        case 10:
            selected = 4;
            break;
        default:
            selected = 0; // 默认选择50%
            break;
    }
    lv_dropdown_set_selected(ui_Dropdown2, selected);
}

// 初始化通道Dropdown
void init_channel() {
    lv_dropdown_set_selected(ui_Dropdown12, channel - 1);
}

// 初始化双击触摸Switch
void init_double_touch() {
    if (double_touch) {
        lv_obj_add_state(ui_Switch3, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_Switch3, LV_STATE_CHECKED);
    }
}

// 初始化双击击打Switch
void init_double_hit() {
    if (double_hit) {
        lv_obj_add_state(ui_Switch4, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_Switch4, LV_STATE_CHECKED);
    }
}

// 初始化鼠标x轴
void init_m_x() {
    lv_dropdown_set_selected(ui_Dropdown3, m_x);
}

// 初始化鼠标y轴
void init_m_y() {
    lv_dropdown_set_selected(ui_Dropdown4, m_y);
}

// 初始化鼠标z轴
void init_m_z() {
    lv_dropdown_set_selected(ui_Dropdown5, m_z);
}

// 初始化Yaw通道Dropdown
void init_yaw_channel() {
    lv_dropdown_set_selected(ui_Dropdown6, yaw_channel - 1);
}

// 初始化Pitch通道Dropdown
void init_pitch_channel() {
    lv_dropdown_set_selected(ui_Dropdown7, pitch_channel - 1);
}

// 初始化Roll通道Dropdown
void init_roll_channel() {
    lv_dropdown_set_selected(ui_Dropdown8, roll_channel - 1);
}

// 初始化Yaw最大值Dropdown
void init_yaw_max() {
    int selected;
    switch (yaw_max) {
        case 180:
            selected = 0;
            break;
        case 270:
            selected = 1;
            break;
        case 360:
            selected = 2;
            break;
        default:
            selected = 0; // 默认180
            break;
    }
    lv_dropdown_set_selected(ui_Dropdown9, selected);
}

// 初始化Pitch最大值Dropdown
void init_pitch_max() {
    int selected;
    switch (pitch_max) {
        case 60:
            selected = 0;
            break;
        case 90:
            selected = 1;
            break;
        default:
            selected = 0; // 默认60
            break;
    }
    lv_dropdown_set_selected(ui_Dropdown10, selected);
}

// 初始化Roll最大值Dropdown
void init_roll_max() {
    int selected;
    switch (roll_max) {
        case 60:
            selected = 0;
            break;
        case 90:
            selected = 1;
            break;
        default:
            selected = 0; // 默认60
            break;
    }
    lv_dropdown_set_selected(ui_Dropdown11, selected);
}

//初始化全部
void init_all() {
    init_brightness();
    init_scr_time();
    init_led_signal();
    init_battery_warning();
    init_bat_warn_value();
    init_channel();
    init_double_touch();
    init_double_hit();
    init_m_x();
    init_m_y();
    init_m_z();
    init_yaw_channel();
    init_pitch_channel();
    init_roll_channel();
    init_yaw_max();
    init_pitch_max();
    init_roll_max();
}

void pairing(lv_event_t * e) {
    if (!isConnected && !isMacSet() && !isWifiWorking) {
        isMacgetting = true;
    }
}

void unpair(lv_event_t * e) {
    if (!isConnected) return;

    Data_sent.Unpairing = true;

    while (!Data_read.Unpairing) {
        debug(1, CRGB::Blue, 500, 1000);
        SentData(0);
        delay(50);
    }

    for (int i = 0; i < 6; i++) {
        receiverAddress[i] = 0;
    }

    save_config();

    Esp_now_off();

    ESP.restart();
}

void imu_calibration(lv_event_t * e) {
    if (!isConnected) return;
    imu_cal = true;
    isEasy_1 = false;
}

void compass_calibration(lv_event_t * e) {
    if (!isConnected) return;
    compass_cal = true;
}

void imu_calibration_handler(bool isEasy) {
    if (!imu_cal) return;

    switch (imuCalState) {
        case IMU_IDLE:
            Data_sent.Zeroing = true;

            if (!isEasy) {
                lv_obj_clear_flag(ui_Panel35, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(ui_Panel32, LV_OBJ_FLAG_CLICKABLE);
                lv_label_set_text(ui_Label37, "校准中...");  // 开始校准时显示
            }

            imuStartTime = millis();
            imuFail = false;
            imuCalState = IMU_WAITING;
            break;

        case IMU_WAITING: {
            if (!isEasy) {
                int elapsed_time = (millis() - imuStartTime) / 1000;
                char sec_timer[20];
                sprintf(sec_timer, "%d s", elapsed_time);
                lv_label_set_text(ui_Label38, sec_timer);
            }

            if (Data_read.Zeroing) {
                if (!isEasy) {
                    lv_label_set_text(ui_Label37, "校准完成!");
                    delay(1000);
                }
                imuCalState = IMU_DONE;
            } else if (!isEasy && (millis() - imuStartTime) / 1000 > 5) {
                lv_label_set_text(ui_Label37, "校准失败!");
                delay(1000);
                imuFail = true;
                imuCalState = IMU_DONE;
            }
            break;
        }

        case IMU_DONE:
            if (!isEasy) {
                lv_obj_add_flag(ui_Panel35, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_Panel32, LV_OBJ_FLAG_CLICKABLE);
                lv_label_set_text(ui_Label37, "校准中...");
            }
            imuCalState = IMU_IDLE;  // Reset the state
            Data_sent.Zeroing = false;
            imu_cal = false;
            break;
    }
}

void compass_calibration_handler() {
    if (!compass_cal) return;

    switch (compassCalState) {
        case COMPASS_IDLE:
            Data_sent.Calibrating = true;
            lv_obj_clear_flag(ui_Panel35, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(ui_Panel32, LV_OBJ_FLAG_CLICKABLE);
            lv_label_set_text(ui_Label37, "校准中...");  // 开始校准时显示
            compassStartTime = millis();
            compassFail = false;
            compassCalState = COMPASS_WAITING;
            break;

        case COMPASS_WAITING: {
            int elapsed_time = (millis() - compassStartTime) / 1000;
            char sec_timer[20];
            sprintf(sec_timer, "%d s", elapsed_time);
            lv_label_set_text(ui_Label38, sec_timer);

            if (Data_read.Calibrating) {
                lv_label_set_text(ui_Label37, "校准完成!");  // 校准成功时显示
                delay(1000);
                compassCalState = COMPASS_DONE;
            } else if (elapsed_time > 15) {
                lv_label_set_text(ui_Label37, "校准失败!");  // 超时时显示
                delay(1000);
                compassFail = true;
                compassCalState = COMPASS_DONE;
            }
            break;
        }

        case COMPASS_DONE:
            lv_obj_add_flag(ui_Panel35, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_Panel32, LV_OBJ_FLAG_CLICKABLE);
            lv_label_set_text(ui_Label37, "校准中...");

            // 重新设置为初始状态
            compassCalState = COMPASS_IDLE;
            compass_cal = false;
            Data_sent.Calibrating = false;
            break;
    }
}