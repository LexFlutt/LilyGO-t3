#include "ui.h"

lv_obj_t *ui_Screen1;
lv_obj_t *ui_TimeLabel;
lv_obj_t *ui_DisplayLabel;
lv_obj_t *ui_BatteryLabel;
lv_obj_t *ui_ImageBackground;

static lv_style_t style_large_text;
static lv_style_t style_medium_text;

void ui_Screen1_screen_init() {
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_style_init(&style_large_text);
    lv_style_set_text_font(&style_large_text, &lv_font_montserrat_48);    

    lv_style_init(&style_medium_text);
    lv_style_set_text_font(&style_medium_text, &lv_font_montserrat_30);
    
    ui_TimeLabel = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_TimeLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_TimeLabel, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_TimeLabel, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_TimeLabel, "00:00");

    ui_DisplayLabel = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_DisplayLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_DisplayLabel, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_DisplayLabel, LV_ALIGN_CENTER);
    lv_obj_set_y(ui_DisplayLabel, -10); 
    lv_label_set_text(ui_DisplayLabel, "100 km/h");
    
    ui_BatteryLabel = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_BatteryLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BatteryLabel, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_BatteryLabel, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_set_y(ui_BatteryLabel, -10);
    lv_label_set_text(ui_BatteryLabel, "Battery: --.- V");

    ui_ImageBackground= lv_img_create(ui_Screen1);
    
    lv_obj_set_width(ui_ImageBackground, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_ImageBackground, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_ImageBackground, LV_ALIGN_CENTER);

    lv_obj_add_style(ui_DisplayLabel, &style_large_text, 0);
    lv_obj_add_style(ui_TimeLabel, &style_medium_text, 0);
}
