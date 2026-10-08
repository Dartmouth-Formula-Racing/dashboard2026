#include "can_decode.h"
#include "vehicle_data.h"

static uint16_t u16_be(const uint8_t *data)
{
    return ((uint16_t)data[0] << 8) | data[1];
}

static int16_t s16_be(const uint8_t *data)
{
    return (int16_t)u16_be(data);
}

void can_decode(uint32_t id, const uint8_t *data, uint8_t len)
{
    if(len < 8)
        return;

    switch(id)
    {
        case 0x750:
            vehicle_data.bms_ok = data[0];
            vehicle_data.imd_ok = data[1];
            vehicle_data.bspd_ok = data[2];
            vehicle_data.bspd_instant = data[3];

            vehicle_data.drive_state = data[4];
            vehicle_data.vehicle_state = data[5];
            vehicle_data.last_failure = (RTDFailure)data[6];
            break;

        case 0x751:
            vehicle_data.rpm =
                s16_be(&data[0]);

            vehicle_data.efficiency =
                s16_be(&data[2]);

            vehicle_data.odometer =
                u16_be(&data[4]);
            break;

        case 0x752:
            vehicle_data.pack_voltage =
                u16_be(&data[0]) / 100.0f;

            vehicle_data.pack_current =
                s16_be(&data[2]) / 10.0f;

            vehicle_data.soc = data[4];
            vehicle_data.soh = data[5];
            break;

        case 0x753:
            vehicle_data.motor_temp_left =
                s16_be(&data[0]) / 10.0f;

            vehicle_data.inverter_temp_left =
                s16_be(&data[2]) / 10.0f;

            vehicle_data.motor_temp_right =
                s16_be(&data[4]) / 10.0f;

            vehicle_data.inverter_temp_right =
                s16_be(&data[6]) / 10.0f;
            break;

        case 0x754:
            vehicle_data.air1_closed = data[0];
            vehicle_data.air2_closed = data[1];
            break;

        case 0x755:
            vehicle_data.throttle =
                u16_be(&data[0]) / 1000.0f;

            vehicle_data.steering_angle_raw =
                u16_be(&data[2]);

            vehicle_data.brake_pressure_raw =
                u16_be(&data[4]);
            break;
    }
}