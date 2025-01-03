#include "ui.h"

lv_obj_t *ui_Screen1;
lv_obj_t *ui_Panel1;
lv_obj_t *ui_Button1;
lv_obj_t *ui_TimeLabel;
lv_obj_t *ui_DisplayLabel;

static lv_style_t style_margin;
static lv_style_t style_large_text;
static lv_style_t style_medium_text;

void ui_Screen1_screen_init() {
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_style_init(&style_large_text);
    lv_style_set_text_font(&style_large_text, &lv_font_montserrat_48);    

    lv_style_init(&style_medium_text);
    lv_style_set_text_font(&style_medium_text, &lv_font_montserrat_30);
    
    lv_style_init(&style_margin);
    lv_style_set_pad_all(&style_margin, 10);
    lv_style_set_border_width(&style_margin, 0);
    lv_style_set_bg_opa(&style_margin, LV_OPA_TRANSP);

    ui_Panel1 = lv_obj_create(ui_Screen1);
    lv_obj_set_size(ui_Panel1, LV_HOR_RES, LV_VER_RES);
    lv_obj_add_style(ui_Panel1, &style_margin, 0);

    ui_TimeLabel = lv_label_create(ui_Panel1);
    lv_obj_set_width(ui_TimeLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_TimeLabel, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_TimeLabel, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_TimeLabel, "00:00");

    ui_DisplayLabel = lv_label_create(ui_Panel1);
    lv_obj_set_width(ui_DisplayLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_DisplayLabel, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_DisplayLabel, LV_ALIGN_CENTER);
    lv_obj_set_y(ui_DisplayLabel, -30); 
    lv_label_set_text(ui_DisplayLabel, "100 km/h");
      
    ui_Button1 = lv_btn_create(ui_Panel1);
    lv_obj_set_width(ui_Button1, 100);
    lv_obj_set_height(ui_Button1, 50);
    lv_obj_set_align(ui_Button1, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_y(ui_Button1, -10); 
    
    lv_obj_t *button_label = lv_label_create(ui_Button1);
    lv_label_set_text(button_label, "Toggle");

    lv_obj_add_style(ui_DisplayLabel, &style_large_text, 0);
    lv_obj_add_style(ui_TimeLabel, &style_medium_text, 0);
}
