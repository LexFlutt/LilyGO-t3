#include "ui.h"

lv_obj_t* ui_Screen1;
lv_obj_t* ui_Panel1;
lv_obj_t* ui_Button1;
lv_obj_t* ui_DisplayLabel;

void ui_Screen1_screen_init() {
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);

    ui_Panel1 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Panel1, 100);
    lv_obj_set_height(ui_Panel1, 50);
    lv_obj_set_align(ui_Panel1, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_Panel1, LV_OBJ_FLAG_SCROLLABLE);

    ui_Button1 = lv_btn_create(ui_Screen1);
    lv_obj_set_width(ui_Button1, 100);
    lv_obj_set_height(ui_Button1, 50);
    lv_obj_set_x(ui_Button1, -107);
    lv_obj_set_y(ui_Button1, 53);
    lv_obj_set_align(ui_Button1, LV_ALIGN_CENTER);
    
    
    ui_DisplayLabel = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_DisplayLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_DisplayLabel, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_DisplayLabel, LV_ALIGN_BOTTOM_MID);

    // Initialize the display with the current time
    lv_label_set_text(ui_DisplayLabel, "Time: 00:00");
}
