SET(SOURCES components/ui_comp_panel1.c
    components/ui_comp.c
    screens/ui_Screen1.c
    screens/ui_Screen2.c
    screens/ui_Screen3.c
    screens/ui_Screen4.c
    screens/ui_Screen5.c
    screens/ui_Screen6.c
    screens/ui_Screen7.c
    screens/ui_Screen8.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_barrety_png.c
    images/ui_img_usb1_png.c
    images/ui_img_signal4_png.c
    images/ui_img_0_png.c
    images/ui_img_1_png.c
    images/ui_img_signal0_png.c
    images/ui_img_signal1_png.c
    images/ui_img_signal2_png.c
    images/ui_img_signal3_png.c
    fonts/ui_font_fangsong.c)

add_library(ui ${SOURCES})
