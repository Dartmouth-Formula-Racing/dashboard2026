#pragma once

#ifdef __cplusplus
extern "C" {
#endif

char *tft_ltoa_compat(long value, char *str, int base);

#ifdef __cplusplus
}
#endif

#ifndef ltoa
#define ltoa tft_ltoa_compat
#endif
