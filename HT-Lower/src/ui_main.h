#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <IT7259Driver.h>
#include "tusb.h"

extern unsigned long lastTouchTime;

extern TFT_eSPI tft;

extern TouchPointData touchData;

extern bool isTouch;

void Ui_setup();
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p );
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data );
void UI_SR1();
void UI_SR4();
void UI_SR7();
void UI_SR8();