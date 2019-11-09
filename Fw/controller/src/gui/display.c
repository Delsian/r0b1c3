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
#include "nrf.h"
#include "ili9341.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "display.h"

#define GRAY            0xC618
#define RED             0xF800
#define BLUE            0x001F

tColor const img[32] = {
        {2,2}, {2,2}, {2,1}, {1,1},
        {1,2}, {2,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,7},
        {1,1}, {1,1}, {3,4}, {5,6},
        {1,1}, {1,1}, {1,1}, {1,1}
        };

tSprite arrow = {
        .width = 8,
        .height = 8,
        .img = img
};

static void DispCursor(uint16_t x, uint16_t y) {
    static uint16_t ox, oy;
    SpriteClear( ox, oy, &arrow);
    SpriteDraw( x, y, &arrow);
    ox = x;
    oy = y;
}

static void DispTouch(const ControlEvent* pEvt) {
    uint16_t *pos = pEvt->ptr16;
    NRF_LOG_INFO("Disp %d %d", pos[0], pos[1]);
    DispCursor(pos[0]/2, pos[1]);
}

void DisplayInit(void)
{
    APP_ERROR_CHECK(ili9341_init());
    ControlRegisterCb(CE_TOUCH, DispTouch);
}

void DispTest(void) {
    static tColor bg[32];
    ClearDisplay();
    arrow.bg = bg;
    DispCursor(150, 150);
}
