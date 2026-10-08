#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RTD_FAIL_NONE = 0,
    RTD_FAIL_DRIVE_LOCKOUT,
    RTD_FAIL_THROTTLE,
    RTD_FAIL_AIR1_OPEN,
    RTD_FAIL_AIR2_OPEN,
    RTD_FAIL_PRECHARGE_TIMEOUT,
    RTD_FAIL_INVALID_STATE,
} RTDFailure;

typedef struct {
    float throttle;

    int16_t rpm;

    float pack_voltage;
    float pack_current;
    uint8_t soc;
    uint8_t soh;

    float motor_temp_left;
    float motor_temp_right;
    float inverter_temp_left;
    float inverter_temp_right;

    uint8_t drive_state;
    uint8_t vehicle_state;

    uint8_t bms_ok;
    uint8_t imd_ok;
    uint8_t bspd_ok;
    uint8_t bspd_instant;

    uint8_t air1_closed;
    uint8_t air2_closed;

    RTDFailure last_failure;
} VehicleData;

extern VehicleData vehicle_data;

#ifdef __cplusplus
}
#endif