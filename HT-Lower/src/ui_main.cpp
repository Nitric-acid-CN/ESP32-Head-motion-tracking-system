#include "ui_main.h"
#include "sensor.h"
#include "wireless.h"
#include "config.h"

#define MAX_DISTANCE 30 // 允许的最大距离变化
#define MIN_TIME_INTERVAL 100 // 允许的最小时间间隔（毫秒）

TouchPointData touchData;

static uint16_t lastXPos = 0;
static uint16_t lastYPos = 0;

int16_t last_x;
int16_t last_y;

bool isTouch = false;

unsigned long lastTouchTime = 0;

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 80;
static const uint16_t screenHeight = 160;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

void Ui_setup() {
    lv_init();

    tft.init();          /* TFT init */

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

    ui_init();

    tusb_init();

    Wire.begin(14, 15, 400000);
}

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ) {
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data ) {
    readTouchEvent(&touchData);

    if (touchData.isNull) {
        data->state = LV_INDEV_STATE_RELEASED;
        isTouch = false;
    } else {
        uint16_t distance = sqrt(pow((touchData.xPos - lastXPos), 2) + pow((touchData.yPos - lastYPos), 2));

        // 判断是否超过阈值
        if (distance <= MAX_DISTANCE || (millis() - lastTouchTime) >= MIN_TIME_INTERVAL) {
            // 更新触摸点
            last_x = touchData.xPos;
            last_y = touchData.yPos;
            data->state = LV_INDEV_STATE_PRESSED; 

            // 更新上一个位置和时间
            lastXPos = touchData.xPos;
            lastYPos = touchData.yPos;
            lastTouchTime = millis();
            isTouch = true;
        } else {
            // 距离变化过大，保持上一个点数据
            data->state = LV_INDEV_STATE_RELEASED;
            isTouch = false;
        }
    }

    data->point.x = last_x;
    data->point.y = last_y;
}

void UI_SR1() {
    static char Bat_rx[20]={0};
    int battery_rx = voltageToCharge(RX_battery_voltage);
    sprintf(Bat_rx, "%.2f", RX_battery_voltage);
    lv_label_set_text(ui_battery, Bat_rx);
    lv_bar_set_value(ui_Bar1, (100 - battery_rx), LV_ANIM_OFF);

    if (tud_mounted()) lv_obj_clear_flag(ui_usb, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(ui_usb, LV_OBJ_FLAG_HIDDEN);

    //连接状态信息
    if (isConnected) {
        lv_obj_clear_flag(ui_Panel5, LV_OBJ_FLAG_HIDDEN);

        static char Bat_tx[20]={0};
        int battery_tx = voltageToCharge(TX_battery_voltage);
        sprintf(Bat_tx, "%.2f", TX_battery_voltage);
        lv_label_set_text(ui_battery2, Bat_tx);
        lv_bar_set_value(ui_Bar2, (100 - battery_tx), LV_ANIM_OFF);

        static char signal_delay[20]={0};
        int quality = esp_now_rssi();
        sprintf(signal_delay, "%d", quality);
        lv_label_set_text(ui_rssi, signal_delay);

        // 根据信号质量设置图片
        if (quality < 10) {
            lv_img_set_src(ui_signal, &ui_img_signal4_png);  // 四格信号
        } else if (quality >= 10) {
            lv_img_set_src(ui_signal, &ui_img_signal3_png);  // 三格信号
        } else if (quality >= 20) {
            lv_img_set_src(ui_signal, &ui_img_signal2_png);  // 二格信号
        } else if (quality >= 30) {
            lv_img_set_src(ui_signal, &ui_img_signal1_png);  // 一格信号
        } else {
            lv_img_set_src(ui_signal, &ui_img_signal0_png);  // 无信号
        }

    } else if (!isMacSet()) {
        lv_obj_add_flag(ui_Panel5, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_Label1, "未配对");
    } else {
        lv_obj_add_flag(ui_Panel5, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_Label1, "未连接");
    }

    //摇杆信息
    static char Yaw_info[20]={0}, Pitch_Roll_info[30]={0};
    sprintf(Yaw_info, "Y: %d", yaw);
    sprintf(Pitch_Roll_info, "P: %.1f R: %.1f", pitch, roll);

    lv_label_set_text(ui_Label4, Yaw_info);
    lv_label_set_text(ui_Label3, Pitch_Roll_info);

    int joystick_x = round((-roll / 90.0) * 31);
    int joystick_y = round((-pitch / 90.0) * 31);

    lv_obj_set_x(ui_joystick, joystick_x);
    lv_obj_set_y(ui_joystick, joystick_y);
}

void UI_SR4() {
    static char Mac[30];
    sprintf(Mac, "%02X:%02X:%02X:%02X:%02X:%02X", receiverAddress[0], receiverAddress[1], receiverAddress[2], receiverAddress[3], receiverAddress[4], receiverAddress[5]);
    lv_label_set_text(ui_Label22, Mac);
}

void UI_SR7() {
    static char ms[10];
    sprintf(ms, "%d", mouse_sens);
    lv_label_set_text(ui_Label44, ms);
}

void UI_SR8() {
    static char yo[10];
    sprintf(yo, "%d", yaw_offset);
    lv_label_set_text(ui_Label54, yo);

    static char po[10];
    sprintf(po, "%d", pitch_offset);
    lv_label_set_text(ui_Label56, po);

    static char ro[10];
    sprintf(ro, "%d", roll_offset);
    lv_label_set_text(ui_Label58, ro);
}