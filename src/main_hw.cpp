#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <mcp_can.h>

#include "lvgl.h"

extern "C" {
#include "dashboard.h"
#include "can_decode.h"
}

#ifndef CAN_CS_PIN
#define CAN_CS_PIN D10
#endif

#ifndef CAN_INT_PIN
#define CAN_INT_PIN D2
#endif

#ifndef DASHBOARD_CAN_BITRATE
#define DASHBOARD_CAN_BITRATE CAN_500KBPS
#endif

#ifndef DASHBOARD_LCD_ROTATION
#define DASHBOARD_LCD_ROTATION 1
#endif

static TFT_eSPI tft = TFT_eSPI();
static MCP_CAN canBus(CAN_CS_PIN);

static uint8_t lvgl_buf1[800 * 16 * 2];

static void lvgl_flush_cb(lv_display_t *display,
                          const lv_area_t *area,
                          uint8_t *px_map)
{
    uint32_t width = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t height = (uint32_t)(area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, width, height);
    tft.pushPixels((uint16_t *)px_map, width * height);
    tft.endWrite();

    lv_display_flush_ready(display);
}

static bool can_init(void)
{
    pinMode(CAN_INT_PIN, INPUT_PULLUP);

    if(canBus.begin(MCP_ANY, DASHBOARD_CAN_BITRATE, MCP_8MHZ) == CAN_OK)
    {
        canBus.setMode(MCP_NORMAL);
        return true;
    }

    if(canBus.begin(MCP_ANY, DASHBOARD_CAN_BITRATE, MCP_16MHZ) == CAN_OK)
    {
        canBus.setMode(MCP_NORMAL);
        return true;
    }

    return false;
}

static void can_poll(void)
{
    if(canBus.checkReceive() != CAN_MSGAVAIL)
        return;

    unsigned long id = 0;
    unsigned char len = 0;
    uint8_t data[8] = {0};

    if(canBus.readMsgBuf(&id, &len, data) != CAN_OK)
        return;

    can_decode((uint32_t)(id & 0x1FFFFFFFUL), data, len);
}

void setup(void)
{
    Serial.begin(115200);

    tft.init();
    tft.setRotation(DASHBOARD_LCD_ROTATION);
    tft.fillScreen(TFT_BLACK);

    lv_init();

    lv_display_t *display = lv_display_create(800, 480);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, lvgl_flush_cb);
    lv_display_set_buffers(display,
                           lvgl_buf1,
                           NULL,
                           sizeof(lvgl_buf1),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    dashboard_create();

    (void)can_init();
}

void loop(void)
{
    can_poll();
    dashboard_update();

    lv_timer_handler();
    delay(5);
}
