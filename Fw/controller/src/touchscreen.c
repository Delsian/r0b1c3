/*
 *  Touch screen reader
 *
 *  Copyright (C) 2019 Eug Krashtan <ekrashtan@n-ix.com>
 *  This file may be distributed under the terms of the GNU GPLv3 license.
 *
 */

#include <stdint.h>
#include "control.h"
#include "boards.h"

void TouchInit(void) {
    nrf_gpio_cfg_output(PIN_TOUCHXL);
    nrf_gpio_pin_set(PIN_TOUCHXL);
    nrf_gpio_cfg_output(PIN_TOUCHXR);
    nrf_gpio_pin_clear(PIN_TOUCHXR);

}
