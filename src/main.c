#include <unistd.h>

#include "lvgl.h"
#include "dashboard.h"
#include "can_decode.h"
#include "vehicle_data.h"

static void simulate_can(void)
{
    uint8_t frame_750[8] = {
        1,
        1,
        1,
        0,
        1,
        0,
        RTD_FAIL_THROTTLE,
        0
    };

    uint8_t frame_751[8] = {
        0x02, 0xEE,
        0x0B, 0xB8,
        0, 0,
        0, 0
    };

    uint8_t frame_752[8] = {
        0xA4, 0x74,
        0x00, 0xA2,
        78,
        97,
        0,
        0
    };

    uint8_t frame_753[8] = {
        0x02, 0x6C,
        0x01, 0xE0,
        0x02, 0x58,
        0x01, 0xFE
    };

    can_decode(
        0x750,
        frame_750,
        8
    );

    can_decode(
        0x751,
        frame_751,
        8
    );

    can_decode(
        0x752,
        frame_752,
        8
    );

    can_decode(
        0x753,
        frame_753,
        8
    );
}

int main(void)
{
    lv_init();

    lv_sdl_window_create(
        800,
        480
    );

    dashboard_create();

    simulate_can();

    while(1)
    {
        dashboard_update();

        lv_timer_handler();

        usleep(10000);
    }

    return 0;
}