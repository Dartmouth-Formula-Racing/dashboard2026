#include "lvgl.h"
#include "dashboard.h"
#include "vehicle_data.h"

static lv_obj_t *state_label;
static lv_obj_t *failure_label;
static lv_obj_t *soc_label;

static lv_obj_t *voltage_label;
static lv_obj_t *current_label;

static lv_obj_t *motor_left_label;
static lv_obj_t *motor_right_label;
static lv_obj_t *inverter_left_label;
static lv_obj_t *inverter_right_label;

static lv_obj_t *rpm_label;

static void make_data_box(lv_obj_t *parent,
                          lv_obj_t **value_label,
                          const char *value,
                          const char *name,
                          int x,
                          int width)
{
    lv_obj_t *box = lv_obj_create(parent);

    lv_obj_set_size(box, width, 100);
    lv_obj_set_pos(box, x, 350);

    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_bg_color(box, lv_color_hex(0x181818), 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_radius(box, 8, 0);

    *value_label = lv_label_create(box);

    lv_label_set_text(*value_label, value);

    lv_obj_set_style_text_font(
        *value_label,
        &lv_font_montserrat_28,
        0
    );

    lv_obj_set_style_text_color(
        *value_label,
        lv_color_hex(0xFFFFFF),
        0
    );

    lv_obj_align(
        *value_label,
        LV_ALIGN_TOP_MID,
        0,
        5
    );

    lv_obj_t *name_label = lv_label_create(box);

    lv_label_set_text(name_label, name);

    lv_obj_set_style_text_color(
        name_label,
        lv_color_hex(0xAAAAAA),
        0
    );

    lv_obj_align(
        name_label,
        LV_ALIGN_BOTTOM_MID,
        0,
        -5
    );
}

static const char *failure_text(RTDFailure failure)
{
    switch(failure)
    {
        case RTD_FAIL_NONE:
            return "";

        case RTD_FAIL_DRIVE_LOCKOUT:
            return "LAST ERROR: DRIVE LOCKOUT";

        case RTD_FAIL_THROTTLE:
            return "LAST ERROR: THROTTLE";

        case RTD_FAIL_AIR1_OPEN:
            return "LAST ERROR: AIR 1 OPEN";

        case RTD_FAIL_AIR2_OPEN:
            return "LAST ERROR: AIR 2 OPEN";

        case RTD_FAIL_PRECHARGE_TIMEOUT:
            return "LAST ERROR: PRECHARGE TIMEOUT";

        case RTD_FAIL_INVALID_STATE:
            return "LAST ERROR: INVALID STATE";

        default:
            return "LAST ERROR: UNKNOWN FAULT";
    }
}

void dashboard_create(void)
{
    lv_obj_t *screen = lv_screen_active();

    lv_obj_set_style_bg_color(
        screen,
        lv_color_hex(0x000000),
        0
    );

    lv_obj_set_style_text_color(
        screen,
        lv_color_hex(0xFFFFFF),
        0
    );

    state_label = lv_label_create(screen);

    lv_label_set_text(
        state_label,
        "NEUTRAL"
    );

    lv_obj_set_style_text_font(
        state_label,
        &lv_font_montserrat_28,
        0
    );

    lv_obj_align(
        state_label,
        LV_ALIGN_TOP_LEFT,
        30,
        20
    );

    soc_label = lv_label_create(screen);

    lv_label_set_text(
        soc_label,
        "SOC 0%"
    );

    lv_obj_set_style_text_font(
        soc_label,
        &lv_font_montserrat_28,
        0
    );

    lv_obj_align(
        soc_label,
        LV_ALIGN_TOP_RIGHT,
        -30,
        20
    );

    failure_label = lv_label_create(screen);

    lv_label_set_text(
        failure_label,
        ""
    );

    lv_obj_set_style_text_color(
        failure_label,
        lv_color_hex(0xFF3333),
        0
    );

    lv_obj_set_style_text_font(
        failure_label,
        &lv_font_montserrat_20,
        0
    );

    lv_obj_align(
        failure_label,
        LV_ALIGN_TOP_MID,
        0,
        70
    );

    rpm_label = lv_label_create(screen);

    lv_label_set_text(
        rpm_label,
        "0 RPM"
    );

    lv_obj_set_style_text_font(
        rpm_label,
        &lv_font_montserrat_48,
        0
    );

    lv_obj_align(
        rpm_label,
        LV_ALIGN_CENTER,
        0,
        -60
    );

    make_data_box(
        screen,
        &voltage_label,
        "0.0 V",
        "PACK",
        10,
        120
    );

    make_data_box(
        screen,
        &current_label,
        "0.0 A",
        "CURRENT",
        140,
        120
    );

    make_data_box(
        screen,
        &motor_left_label,
        "0 C",
        "MOTOR L",
        270,
        120
    );

    make_data_box(
        screen,
        &motor_right_label,
        "0 C",
        "MOTOR R",
        400,
        120
    );

    make_data_box(
        screen,
        &inverter_left_label,
        "0 C",
        "INV L",
        530,
        120
    );

    make_data_box(
        screen,
        &inverter_right_label,
        "0 C",
        "INV R",
        660,
        120
    );
}

void dashboard_update(void)
{
    lv_label_set_text_fmt(
        soc_label,
        "SOC %u%%",
        vehicle_data.soc
    );

    lv_label_set_text_fmt(
        rpm_label,
        "%d RPM",
        vehicle_data.rpm
    );

    lv_label_set_text_fmt(
        voltage_label,
        "%.1f V",
        vehicle_data.pack_voltage
    );

    lv_label_set_text_fmt(
        current_label,
        "%.1f A",
        vehicle_data.pack_current
    );

    lv_label_set_text_fmt(
        motor_left_label,
        "%.1f C",
        vehicle_data.motor_temp_left
    );

    lv_label_set_text_fmt(
        motor_right_label,
        "%.1f C",
        vehicle_data.motor_temp_right
    );

    lv_label_set_text_fmt(
        inverter_left_label,
        "%.1f C",
        vehicle_data.inverter_temp_left
    );

    lv_label_set_text_fmt(
        inverter_right_label,
        "%.1f C",
        vehicle_data.inverter_temp_right
    );

    lv_label_set_text(
        failure_label,
        failure_text(vehicle_data.last_failure)
    );

    switch(vehicle_data.drive_state)
    {
        case 0:
            lv_label_set_text(state_label, "NEUTRAL");
            break;

        case 1:
            lv_label_set_text(state_label, "DRIVE");
            break;

        case 2:
            lv_label_set_text(state_label, "REVERSE");
            break;

        default:
            lv_label_set_text(state_label, "UNKNOWN");
            break;
    }

    if(vehicle_data.last_failure != RTD_FAIL_NONE)
    {
        lv_obj_set_style_text_color(
            state_label,
            lv_color_hex(0xFF3333),
            0
        );
    }
    else
    {
        lv_obj_set_style_text_color(
            state_label,
            lv_color_hex(0x00FF55),
            0
        );
    }
}