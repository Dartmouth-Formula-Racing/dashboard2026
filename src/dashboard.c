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

static lv_obj_t *steering_bar;
static lv_obj_t *steering_label;

static lv_obj_t *throttle_bar;
static lv_obj_t *throttle_label;

static lv_obj_t *brake_bar;
static lv_obj_t *brake_label;

static lv_obj_t *air1_box;
static lv_obj_t *air1_label;
static lv_obj_t *air2_box;
static lv_obj_t *air2_label;

static lv_obj_t *rpm_label;

static int32_t clamp_int32(int32_t value, int32_t min, int32_t max)
{
    if(value < min)
        return min;

    if(value > max)
        return max;

    return value;
}

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

static void make_status_box(lv_obj_t *parent,
                            lv_obj_t **box_label,
                            lv_obj_t **text_label,
                            const char *name,
                            int x,
                            int y)
{
    lv_obj_t *box = lv_obj_create(parent);

    lv_obj_set_size(box, 150, 42);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_bg_color(box, lv_color_hex(0x202020), 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_radius(box, 10, 0);

    lv_obj_t *label = lv_label_create(box);

    lv_label_set_text_fmt(label, "%s: --", name);

    lv_obj_set_style_text_font(
        label,
        &lv_font_montserrat_18,
        0
    );

    lv_obj_set_style_text_color(
        label,
        lv_color_hex(0xFFFFFF),
        0
    );

    lv_obj_center(label);

    *box_label = box;
    *text_label = label;
}

static void make_vertical_gauge(lv_obj_t *parent,
                                lv_obj_t **label_out,
                                lv_obj_t **bar_out,
                                const char *name,
                                int x,
                                int y,
                                int height,
                                int32_t max_value)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, name);

    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xCCCCCC), 0);

    lv_obj_set_size(label, 120, 25);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_set_style_transform_pivot_x(label, 60, 0);
    lv_obj_set_style_transform_pivot_y(label, 12, 0);
    lv_obj_set_style_transform_rotation(label, 2700, 0);

    lv_obj_set_pos(label, x - 75, y + height / 2 - 12);

    lv_obj_t *bar = lv_bar_create(parent);

    lv_obj_set_size(bar, 20, height);
    lv_obj_set_pos(bar, x, y);

    lv_bar_set_range(bar, 0, max_value);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_bar_set_mode(bar, LV_BAR_MODE_NORMAL);
    lv_bar_set_orientation(bar, LV_BAR_ORIENTATION_VERTICAL);

    lv_obj_set_style_bg_color(bar, lv_color_hex(0x2E2E2E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x0F6B2F), LV_PART_INDICATOR);

    *label_out = label;
    *bar_out = bar;
}

static void make_steering_indicator(lv_obj_t *parent,
                                    lv_obj_t **label_out,
                                    lv_obj_t **bar_out)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "STEERING");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 4);

    lv_obj_t *bar = lv_bar_create(parent);
    lv_obj_set_size(bar, 220, 16);
    lv_obj_set_pos(bar, 290, 30);
    lv_bar_set_range(bar, -100, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_bar_set_mode(bar, LV_BAR_MODE_SYMMETRICAL);

    lv_obj_set_style_bg_color(bar, lv_color_hex(0x2E2E2E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x1E90FF), LV_PART_INDICATOR);

    lv_obj_t *center_mark = lv_obj_create(parent);
    lv_obj_set_size(center_mark, 2, 22);
    lv_obj_set_pos(center_mark, 399, 27);
    lv_obj_set_style_radius(center_mark, 0, 0);
    lv_obj_set_style_border_width(center_mark, 0, 0);
    lv_obj_set_style_bg_color(center_mark, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_bg_opa(center_mark, LV_OPA_COVER, 0);

    *label_out = label;
    *bar_out = bar;
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

    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);

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
        130
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

    make_status_box(
        screen,
        &air1_box,
        &air1_label,
        "AIR 1",
        10,
        190
    );

    make_status_box(
        screen,
        &air2_box,
        &air2_label,
        "AIR 2",
        10,
        245
    );

    make_steering_indicator(
        screen,
        &steering_label,
        &steering_bar
    );

    make_vertical_gauge(
        screen,
        &throttle_label,
        &throttle_bar,
        "THROTTLE",
        680,
        160,
        170,
        1000
    );

    make_vertical_gauge(
        screen,
        &brake_label,
        &brake_bar,
        "BRAKE",
        755,
        160,
        170,
        4095
    );
}

static void update_status_box(lv_obj_t *box,
                              lv_obj_t *label,
                              const char *name,
                              uint8_t closed)
{
    if(closed)
    {
        lv_obj_set_style_bg_color(box, lv_color_hex(0x0F6B2F), 0);
        lv_label_set_text_fmt(label, "%s: CLOSED", name);
    }
    else
    {
        lv_obj_set_style_bg_color(box, lv_color_hex(0x8A1F1F), 0);
        lv_label_set_text_fmt(label, "%s: OPEN", name);
    }
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

    lv_bar_set_value(
        throttle_bar,
        (int32_t)(vehicle_data.throttle * 1000.0f),
        LV_ANIM_OFF
    );

    lv_bar_set_value(
        brake_bar,
        vehicle_data.brake_pressure_raw,
        LV_ANIM_OFF
    );

    int32_t steering_percent =
        ((int32_t)vehicle_data.steering_angle_raw - 2048) * 100 / 2048;

    steering_percent = clamp_int32(steering_percent, -100, 100);

    lv_bar_set_value(
        steering_bar,
        steering_percent,
        LV_ANIM_OFF
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

    update_status_box(
        air1_box,
        air1_label,
        "AIR 1",
        vehicle_data.air1_closed
    );

    update_status_box(
        air2_box,
        air2_label,
        "AIR 2",
        vehicle_data.air2_closed
    );
}