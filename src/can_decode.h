#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void can_decode(uint32_t id, const uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif