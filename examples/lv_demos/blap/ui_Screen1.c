#include "ui.h"

lv_obj_t* ui_Screen1;
lv_obj_t* ui_Panel1;
lv_obj_t* ui_Button1;
lv_obj_t* ui_DisplayLabel;

int display_mode = 0; // 0: Time, 1: Speed, 2: Altitude

void toggle_display() {
    display_mode = (display_mode + 1) % 3;

    if (display_mode == 0) {
        lv_label_set_text(ui_DisplayLabel, "Time: 00:00");
    } else if (display_mode == 1) {
        lv_label_set_text(ui_DisplayLabel, "Speed: 100 km/h");
    } else if (display_mode == 2) {
        lv_label_set_text(ui_DisplayLabel, "Altitude: 5000 ft");
    }
}

void ui_Screen1_screen_init() {
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);  // Flags

    ui_Panel1 = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_Panel1, 100);
    lv_obj_set_height(ui_Panel1, 50);
    lv_obj_set_align(ui_Panel1, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_Panel1, LV_OBJ_FLAG_SCROLLABLE);  // Flags

    ui_Button1 = lv_btn_create(ui_Screen1);
    lv_obj_set_width(ui_Button1, 100);
    lv_obj_set_height(ui_Button1, 50);
    lv_obj_set_x(ui_Button1, -107);
    lv_obj_set_y(ui_Button1, 53);
    lv_obj_set_align(ui_Button1, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(ui_Button1, toggle_display, LV_EVENT_CLICKED, NULL);

    // Create the label for displaying time, speed, and altitude
    ui_DisplayLabel = lv_label_create(ui_Screen1);
    lv_obj_set_width(ui_DisplayLabel, LV_SIZE_CONTENT);  // 1
    lv_obj_set_height(ui_DisplayLabel, LV_SIZE_CONTENT);  // 1
    lv_obj_set_align(ui_DisplayLabel, LV_ALIGN_BOTTOM_MID);  // Position at the bottom middle

    // Initialize the display with the current time
    lv_label_set_text(ui_DisplayLabel, "Time: 00:00");
}
