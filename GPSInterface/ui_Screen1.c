#include "ui.h"

lv_obj_t *ui_Screen1;
lv_obj_t *ui_SpeedPanel;
lv_obj_t *ui_SpeedLabel;
lv_obj_t *ui_SpeedUnitLabel;
lv_obj_t *ui_InfoPanel;
lv_obj_t *ui_TimeLabel;
lv_obj_t *ui_DisplayLabel;
lv_obj_t *ui_BatteryLabel;

static lv_style_t style_margin;
static lv_style_t style_large_text;
static lv_style_t style_medium_text;
static lv_style_t style_small_text;

void ui_Screen1_screen_init() {
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_style_init(&style_large_text);
    lv_style_set_text_font(&style_large_text, &lv_font_montserrat_48);    

    lv_style_init(&style_medium_text);
    lv_style_set_text_font(&style_medium_text, &lv_font_montserrat_30);
    
    lv_style_init(&style_small_text);
    lv_style_set_text_font(&style_small_text, &lv_font_montserrat_20);

    lv_style_init(&style_margin);
    lv_style_set_pad_all(&style_margin, 10);
    lv_style_set_border_width(&style_margin, 0);
    lv_style_set_bg_opa(&style_margin, LV_OPA_TRANSP);

    // Speed Panel
    ui_SpeedPanel = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_SpeedPanel, lv_pct(40));
    lv_obj_set_height(ui_SpeedPanel, lv_pct(100));
    lv_obj_align(ui_SpeedPanel, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_style(ui_SpeedPanel, &style_margin, 0);

    ui_SpeedLabel = lv_label_create(ui_SpeedPanel);
    lv_obj_align(ui_SpeedLabel, LV_ALIGN_CENTER, 0, -40);
    lv_obj_add_style(ui_SpeedLabel, &style_large_text, 0);
    lv_label_set_text(ui_SpeedLabel, "100");

    ui_SpeedUnitLabel = lv_label_create(ui_SpeedPanel);
    lv_obj_align(ui_SpeedUnitLabel, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_style(ui_SpeedUnitLabel, &style_small_text, 0);
    lv_label_set_text(ui_SpeedUnitLabel, "km/h");

    // Info Panel
    ui_InfoPanel = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_InfoPanel, lv_pct(60));
    lv_obj_set_height(ui_SpeedPanel, lv_pct(100));
    lv_obj_align(ui_InfoPanel, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_style(ui_InfoPanel, &style_margin, 0);

    ui_TimeLabel = lv_label_create(ui_InfoPanel);
    lv_obj_align(ui_TimeLabel, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_add_style(ui_TimeLabel, &style_medium_text, 0);
    lv_label_set_text(ui_TimeLabel, "00:00");

    ui_DisplayLabel = lv_label_create(ui_InfoPanel);
    lv_obj_align(ui_DisplayLabel, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_style(ui_DisplayLabel, &style_large_text, 0);
    lv_label_set_text(ui_DisplayLabel, "Altitude");

    ui_BatteryLabel = lv_label_create(ui_InfoPanel);
    lv_obj_align(ui_BatteryLabel, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_label_set_text(ui_BatteryLabel, "Battery: --.- V");
}
