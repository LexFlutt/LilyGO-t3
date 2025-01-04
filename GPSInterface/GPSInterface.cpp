#include <Arduino.h>
#include <WiFi.h>
#include <TouchDrvCSTXXX.hpp>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>
#include <SPIFFS.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>

#include "pin_config.h"
#include "lv_conf.h"
#include "lvgl.h"
#include "Wire.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "ui.h"

HardwareSerial gpsSerial(1);
AsyncWebServer server(80);

const char *ssid = "Flutt";
const char *password = "Buddies1";
const char *ntpServer = "time.nist.gov";
const long gmtOffset_sec = -25200;
const int daylightOffset_sec = 3600;

const double MAX_DISTANCE = 0.01;
double prevLat = -1;
double prevLng = -1;

File gpxFile;

#define BUTTON_PIN 14
#define BATTERY_PIN 4

TouchDrvCSTXXX touch;
int16_t x[5], y[5];

esp_lcd_panel_io_handle_t io_handle = NULL;
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t *lv_disp_buf;
static bool is_initialized_lvgl = false;
TinyGPSPlus gps;
int displayMode = 0;
char timeStr[64];
char altitudeStr[32];
char speedStr[32];
char batteryStr[32];
char buttonLabelStr[32];

bool isDisplayOn = true;
int fileCounter = 1;

typedef struct
{
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},

};

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    if (is_initialized_lvgl)
    {
        lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
        lv_disp_flush_ready(disp_driver);
    }
    return false;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

static void lv_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (touch.getPoint(x, y, touch.getSupportTouchPoint()))
    {
        data->point.x = *x;
        data->point.y = *y;
        data->state = LV_INDEV_STATE_PR;
    }
    else
        data->state = LV_INDEV_STATE_REL;
}
esp_lcd_panel_handle_t panel_handle = NULL;

void writeGpxFileHeader()
{
    gpxFile.print("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    gpxFile.print("<gpx version=\"1.1\" creator=\"Alex\">\n");
    gpxFile.print("<trk>\n");
    gpxFile.print("<name>GPS Track</name>\n");
    gpxFile.print("<trkseg>\n");
}

bool isGpsDataValid(double lat, double lng) {
    if (lat == 0 || lng == 0) {
        Serial.println("Invalid GPS data: Latitude or Longitude is zero.");
        return false;
    }
    double deltaLat = abs(lat - prevLat);
    double deltaLng = abs(lng - prevLng);

    if ((prevLat != -1 && prevLng != -1) && (deltaLat > MAX_DISTANCE || deltaLng > MAX_DISTANCE)) {
        Serial.print("Invalid GPS data: Distance is too large from the previous point.\n");
        Serial.print("Current GPS: lat = ");
        Serial.print(lat, 6);
        Serial.print(", lng = ");
        Serial.println(lng, 6);
        Serial.print("Previous GPS: lat = ");
        Serial.print(prevLat, 6);
        Serial.print(", lng = ");
        Serial.println(prevLng, 6);
        return false;
    }

    prevLat = lat;
    prevLng = lng;

    return true;
}


void InitGpsFile()
{
    String latestFileName = "/gps_data" + String(fileCounter - 1) + ".gpx";
    if (SPIFFS.exists(latestFileName))
    {
        gpxFile = SPIFFS.open(latestFileName, FILE_APPEND);
        if (!gpxFile)
        {
            Serial.println("Failed to open GPX file for appending");
            return;
        }
    }
    else
    {
        gpxFile = SPIFFS.open(latestFileName, FILE_WRITE);
        if (!gpxFile)
        {
            Serial.println("Failed to open GPX file for writing");
            return;
        }
        writeGpxFileHeader();
    }
}

void loadFileCounter()
{
    if (SPIFFS.exists("/fileCounter.txt"))
    {
        File file = SPIFFS.open("/fileCounter.txt", FILE_READ);
        if (file)
        {
            fileCounter = file.parseInt();
            file.close();
        }
    }
}

void saveFileCounter()
{
    File file = SPIFFS.open("/fileCounter.txt", FILE_WRITE);
    if (file)
    {
        file.println(fileCounter);
        file.close();
    }
}

void listFiles(AsyncWebServerRequest *request)
{
    String output = "<html><body><h2>Files on SPIFFS</h2><ul>";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        output += "<li>" + String(file.name()) + " - " + String(file.size()) + " bytes";
        output += " <a href=\"/delete?file=" + String(file.name()) + "\">Delete</a>";
        output += " <a href=\"/download?file=" + String(file.name()) + "\">Download</a></li>";
        file = root.openNextFile();
    }
    output += "</ul>";
    output += "<form action=\"/create\" method=\"GET\"><button type=\"submit\">Create New File</button></form>";
    output += "</body></html>";
    request->send(200, "text/html", output);
}

void deleteFile(AsyncWebServerRequest *request)
{
    if (request->hasParam("file"))
    {
        String filename = "/" + request->getParam("file")->value();
        if (SPIFFS.remove(filename))
        {
            request->send(200, "text/plain", "File deleted successfully");
        }
        else
        {
            request->send(500, "text/plain", "File deletion failed");
        }
    }
    else
    {
        request->send(400, "text/plain", "File parameter missing");
    }
}

void downloadFile(AsyncWebServerRequest *request)
{
    if (request->hasParam("file"))
    {
        String filename = "/" + request->getParam("file")->value();
        request->send(SPIFFS, filename, "application/octet-stream");
    }
    else
    {
        request->send(400, "text/plain", "File parameter missing");
    }
}

void createFile(AsyncWebServerRequest *request)
{
    String fileName = "/gps_data" + String(fileCounter) + ".gpx";
    gpxFile = SPIFFS.open(fileName, FILE_WRITE);
    if (gpxFile)
    {
        writeGpxFileHeader();
        gpxFile.close();
        fileCounter++;
        saveFileCounter();
        request->send(200, "text/plain", "File created: " + fileName);
    }
    else
    {
        request->send(500, "text/plain", "File creation failed");
    }
    InitGpsFile();
}

void SaveGpxData()
{
    if (!gps.time.isValid() || !gps.date.isValid())
    {
        Serial.println("Invalid GPS time or date");
        return;
    }

    if (isGpsDataValid(gps.location.lat(), gps.location.lng()))
    {
        char timeStr[64];
        snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02dT%02d:%02d:%02dZ",
                 gps.date.year(),
                 gps.date.month(),
                 gps.date.day(),
                 gps.time.hour(),
                 gps.time.minute(),
                 gps.time.second());

        gpxFile.print("<trkpt lat=\"");
        gpxFile.print(gps.location.lat(), 6);
        gpxFile.print("\" lon=\"");
        gpxFile.print(gps.location.lng(), 6);
        gpxFile.print("\">\n");
        gpxFile.print("<ele>");
        gpxFile.print(gps.altitude.meters());
        gpxFile.print("</ele>\n");
        gpxFile.print("<time>");
        gpxFile.print(timeStr);
        gpxFile.print("</time>\n");
        gpxFile.print("</trkpt>\n");

        gpxFile.flush();
    }
}

void GetGpsData()
{
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());
    }

    int satellites = gps.satellites.value();
    float altitude = gps.altitude.feet();
    float speed = gps.speed.kmph();

    snprintf(altitudeStr, sizeof(altitudeStr), "%.1f ft", altitude);
    snprintf(speedStr, sizeof(speedStr), "%.1f km/h", speed);
}

void connectToWiFi()
{
    int retries = 0;
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED && retries < 20)
    {
        retries++;
        delay(500);
    }
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String output = "<html><body><h2>Menu</h2><ul>";        
        output += "<li><a href=\"/list\">List Files</a></li>";
        output += "<li><a href=\"/create\">Create New File</a></li>";
        output += "</ul></body></html>";
        request->send(200, "text/html", output); });

    server.on("/list", HTTP_GET, listFiles);
    server.on("/delete", HTTP_GET, deleteFile);
    server.on("/download", HTTP_GET, downloadFile);
    server.on("/create", HTTP_GET, createFile);

    server.begin();
}

void toggleDisplay()
{
    if (isDisplayOn)
    {
        esp_lcd_panel_disp_off(panel_handle, true);
        ledcWrite(0, 0);
    }
    else
    {
        esp_lcd_panel_disp_off(panel_handle, false);
        ledcWrite(0, 255);
    }
    isDisplayOn = !isDisplayOn;
}

void IRAM_ATTR buttonISR()
{
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 200)
    {
        toggleDisplay();
    }
    last_interrupt_time = interrupt_time;
}
void RefreshDisplay()
{
    if (!gps.time.isValid())
    {
        return;
    }

    int hour = gps.time.hour();
    int minute = gps.time.minute();
    int second = gps.time.second();

    hour -= 7;
    if (hour < 0)
    {
        hour += 24;
    }

    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hour, minute, second);
    lv_label_set_text(ui_TimeLabel, timeStr);

    if (displayMode == 0)
    {
        lv_label_set_text(ui_DisplayLabel, speedStr);
        snprintf(buttonLabelStr, sizeof(buttonLabelStr), "Altitude");
    }
    else if (displayMode == 1)
    {
        lv_label_set_text(ui_DisplayLabel, altitudeStr);
        snprintf(buttonLabelStr, sizeof(buttonLabelStr), "Speed");
    }
    lv_label_set_text(button_label, buttonLabelStr);

    float batteryVoltage = analogRead(BATTERY_PIN) * (3.3 / 4095.0) * 2;
    snprintf(batteryStr, sizeof(batteryStr), "Battery: %.1f V", batteryVoltage);
    lv_label_set_text(ui_BatteryLabel, batteryStr);
}

void btn_event_handler(lv_event_t *e)
{
    displayMode = !displayMode;
    RefreshDisplay();
}

void InitScreen()
{
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    pinMode(PIN_LCD_RD, OUTPUT);
    digitalWrite(PIN_LCD_RD, HIGH);
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = PIN_LCD_DC,
        .wr_gpio_num = PIN_LCD_WR,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .data_gpio_nums =
            {
                PIN_LCD_D0,
                PIN_LCD_D1,
                PIN_LCD_D2,
                PIN_LCD_D3,
                PIN_LCD_D4,
                PIN_LCD_D5,
                PIN_LCD_D6,
                PIN_LCD_D7,
            },
        .bus_width = 8,
        .max_transfer_bytes = LVGL_LCD_BUF_SIZE * sizeof(uint16_t),
    };
    esp_lcd_new_i80_bus(&bus_config, &i80_bus);

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = PIN_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 20,
        .on_color_trans_done = notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels =
            {
                .dc_idle_level = 0,
                .dc_cmd_level = 0,
                .dc_dummy_level = 0,
                .dc_data_level = 1,
            },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_LCD_RES,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true);

    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true);
    // the gap is LCD panel specific, even panels with the same driver IC, can
    // have different gap value
    esp_lcd_panel_set_gap(panel_handle, 0, 35);

    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++)
    {
        esp_lcd_panel_io_tx_param(io_handle, lcd_st7789v[i].cmd, lcd_st7789v[i].data, lcd_st7789v[i].len & 0x7f);
        if (lcd_st7789v[i].len & 0x80)
            delay(120);
    }

    ledcSetup(0, 10000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    for (uint8_t i = 0; i < 0xAA; i++)
    {
        ledcWrite(0, i);
        delay(2);
    }

    lv_init();
    lv_disp_buf = (lv_color_t *)heap_caps_malloc(LVGL_LCD_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    lv_disp_draw_buf_init(&disp_buf, lv_disp_buf, NULL, LVGL_LCD_BUF_SIZE);

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);

    touch.setPins(BOARD_TOUCH_RST, BOARD_TOUCH_IRQ);

    if (!touch.begin(Wire, CST328_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL))
    {
        if (!touch.begin(Wire, CST816_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL))
        {
            while (1)
            {
                delay(500);
            }
        }
    }

    touch.setMaxCoordinates(320, 170);
    touch.setMirrorXY(true, false);
    touch.setSwapXY(true);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lv_touchpad_read;
    lv_indev_drv_register(&indev_drv);
    is_initialized_lvgl = true;

    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_Screen1_screen_init();
    lv_disp_load_scr(ui_Screen1);
}

void setup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, 12, 13);
    if (!SPIFFS.begin(true))
    {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }
    loadFileCounter();
    InitScreen();
    lv_obj_add_event_cb(ui_Button1, btn_event_handler, LV_EVENT_CLICKED, NULL);
    connectToWiFi();
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    InitGpsFile();
}

void loop()
{
    lv_task_handler();
    delay(5);

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000)
    {
        GetGpsData();
        SaveGpxData();
        RefreshDisplay();
        lastUpdate = millis();
    }
}
