#include <Arduino.h>
#include <SPI.h>

#include "lvgl.h"

#if defined(DASHBOARD_USE_TFT_ESPI)
#define DASHBOARD_USE_LOVYANGFX 0
#elif defined(ARDUINO_ARCH_ESP32)
#define DASHBOARD_USE_LOVYANGFX 1
#else
#define DASHBOARD_USE_LOVYANGFX 0
#endif

#if defined(ARDUINO_ARCH_ESP32) && !defined(DASHBOARD_USE_MCP_CAN)
#define DASHBOARD_USE_TWAI 1
#include <driver/twai.h>
#else
#define DASHBOARD_USE_TWAI 0
#include <mcp_can.h>
#endif

#if DASHBOARD_USE_LOVYANGFX
#include <Wire.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

class DashboardDisplay : public lgfx::LGFX_Device
{
public:
    lgfx::Bus_RGB bus;
    lgfx::Panel_RGB panel;

    DashboardDisplay()
    {
        auto panel_cfg = panel.config();
        panel_cfg.memory_width = 800;
        panel_cfg.memory_height = 480;
        panel_cfg.panel_width = 800;
        panel_cfg.panel_height = 480;
        panel_cfg.offset_x = 0;
        panel_cfg.offset_y = 0;
        panel.config(panel_cfg);

        auto panel_detail_cfg = panel.config_detail();
        panel_detail_cfg.use_psram = 1;
        panel.config_detail(panel_detail_cfg);

        auto bus_cfg = bus.config();
        bus_cfg.panel = &panel;
        bus_cfg.pin_d0 = GPIO_NUM_21;
        bus_cfg.pin_d1 = GPIO_NUM_47;
        bus_cfg.pin_d2 = GPIO_NUM_48;
        bus_cfg.pin_d3 = GPIO_NUM_45;
        bus_cfg.pin_d4 = GPIO_NUM_38;
        bus_cfg.pin_d5 = GPIO_NUM_9;
        bus_cfg.pin_d6 = GPIO_NUM_10;
        bus_cfg.pin_d7 = GPIO_NUM_11;
        bus_cfg.pin_d8 = GPIO_NUM_12;
        bus_cfg.pin_d9 = GPIO_NUM_13;
        bus_cfg.pin_d10 = GPIO_NUM_14;
        bus_cfg.pin_d11 = GPIO_NUM_7;
        bus_cfg.pin_d12 = GPIO_NUM_17;
        bus_cfg.pin_d13 = GPIO_NUM_18;
        bus_cfg.pin_d14 = GPIO_NUM_3;
        bus_cfg.pin_d15 = GPIO_NUM_46;
        bus_cfg.pin_henable = GPIO_NUM_42;
        bus_cfg.pin_vsync = GPIO_NUM_41;
        bus_cfg.pin_hsync = GPIO_NUM_40;
        bus_cfg.pin_pclk = GPIO_NUM_39;
        bus_cfg.freq_write = 18000000;
        bus_cfg.hsync_polarity = 1;
        bus_cfg.hsync_pulse_width = 4;
        bus_cfg.hsync_back_porch = 8;
        bus_cfg.hsync_front_porch = 8;
        bus_cfg.vsync_polarity = 1;
        bus_cfg.vsync_pulse_width = 4;
        bus_cfg.vsync_back_porch = 8;
        bus_cfg.vsync_front_porch = 8;
        bus_cfg.pclk_idle_high = 1;
        bus.config(bus_cfg);

        panel.setBus(&bus);
        setPanel(&panel);
    }
};
#else
#include <TFT_eSPI.h>
#endif

extern "C" {
#include "dashboard.h"
#include "can_decode.h"
}

#ifndef DASHBOARD_ENABLE_CAN
#define DASHBOARD_ENABLE_CAN 1
#endif

#if DASHBOARD_USE_TWAI
#ifndef DASHBOARD_TWAI_TX_PIN
#define DASHBOARD_TWAI_TX_PIN GPIO_NUM_20
#endif

#ifndef DASHBOARD_TWAI_RX_PIN
#define DASHBOARD_TWAI_RX_PIN GPIO_NUM_19
#endif
#else
#ifndef CAN_CS_PIN
#define CAN_CS_PIN GPIO_NUM_15
#endif

#ifndef CAN_INT_PIN
#define CAN_INT_PIN GPIO_NUM_16
#endif

#ifndef DASHBOARD_CAN_BITRATE
#define DASHBOARD_CAN_BITRATE CAN_500KBPS
#endif
#endif

#ifndef DASHBOARD_LCD_ROTATION
#if DASHBOARD_USE_LOVYANGFX
#define DASHBOARD_LCD_ROTATION 0
#else
#define DASHBOARD_LCD_ROTATION 1
#endif
#endif

#if DASHBOARD_USE_LOVYANGFX
static DashboardDisplay tft;
#else
static TFT_eSPI tft = TFT_eSPI();
#endif
#if DASHBOARD_ENABLE_CAN
#if !DASHBOARD_USE_TWAI
static MCP_CAN canBus(CAN_CS_PIN);
#endif
static bool can_ready = false;
#endif

static uint8_t lvgl_buf1[800 * 16 * 2];

#if DASHBOARD_USE_LOVYANGFX
static bool elecrow_display_power_init(void)
{
    Wire.begin(15, 16);

    for(uint8_t attempt = 0; attempt < 10; ++attempt)
    {
        Wire.beginTransmission(0x30);
        if(Wire.endTransmission() == 0)
        {
            Wire.beginTransmission(0x30);
            Wire.write(0x10);
            if(Wire.endTransmission() == 0)
                return true;

            Serial.println("Elecrow display backlight command failed");
            return false;
        }

        Wire.beginTransmission(0x30);
        Wire.write(0x19);
        Wire.endTransmission();

        pinMode(1, OUTPUT);
        digitalWrite(1, LOW);
        delay(120);
        pinMode(1, INPUT);
        delay(100);
    }

    Serial.println("Elecrow display controller not found at I2C address 0x30");
    return false;
}
#endif

static void lvgl_flush_cb(lv_display_t *display,
                          const lv_area_t *area,
                          uint8_t *px_map)
{
    uint32_t width = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t height = (uint32_t)(area->y2 - area->y1 + 1);

#if DASHBOARD_USE_LOVYANGFX
    tft.pushImage(area->x1,
                  area->y1,
                  width,
                  height,
                  reinterpret_cast<lgfx::rgb565_t *>(px_map));
#else
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, width, height);
    tft.pushPixels((uint16_t *)px_map, width * height);
    tft.endWrite();
#endif

    lv_display_flush_ready(display);
}

static uint32_t lvgl_tick_cb(void)
{
    return (uint32_t)millis();
}

static bool can_init(void)
{
#if DASHBOARD_ENABLE_CAN
#if DASHBOARD_USE_TWAI
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)DASHBOARD_TWAI_TX_PIN,
        (gpio_num_t)DASHBOARD_TWAI_RX_PIN,
        TWAI_MODE_NORMAL
    );
    g_config.rx_queue_len = 32;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if(twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
    {
        Serial.println("TWAI driver install failed");
        return false;
    }

    if(twai_start() != ESP_OK)
    {
        Serial.println("TWAI start failed");
        return false;
    }

    can_ready = true;
    return true;
#else
    pinMode(CAN_INT_PIN, INPUT_PULLUP);

    if(canBus.begin(MCP_ANY, DASHBOARD_CAN_BITRATE, MCP_8MHZ) == CAN_OK)
    {
        canBus.setMode(MCP_NORMAL);
        can_ready = true;
        return true;
    }

    if(canBus.begin(MCP_ANY, DASHBOARD_CAN_BITRATE, MCP_16MHZ) == CAN_OK)
    {
        canBus.setMode(MCP_NORMAL);
        can_ready = true;
        return true;
    }
#endif
#endif

    return false;
}

static void can_poll(void)
{
#if DASHBOARD_ENABLE_CAN
    if(!can_ready)
        return;

#if DASHBOARD_USE_TWAI
    twai_message_t msg;
    while(twai_receive(&msg, 0) == ESP_OK)
    {
        if(msg.rtr)
            continue;

        can_decode(msg.identifier, msg.data, msg.data_length_code);
    }
#else
    if(canBus.checkReceive() != CAN_MSGAVAIL)
        return;

    unsigned long id = 0;
    unsigned char len = 0;
    uint8_t data[8] = {0};

    if(canBus.readMsgBuf(&id, &len, data) != CAN_OK)
        return;

    can_decode((uint32_t)(id & 0x1FFFFFFFUL), data, len);
#endif
#endif
}

void setup(void)
{
    Serial.begin(115200);

#if DASHBOARD_USE_LOVYANGFX
    if(!elecrow_display_power_init())
        Serial.println("Continuing with display initialization");
#endif

    tft.init();
    tft.setRotation(DASHBOARD_LCD_ROTATION);
    tft.fillScreen(TFT_BLACK);

    lv_init();
    lv_tick_set_cb(lvgl_tick_cb);

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
