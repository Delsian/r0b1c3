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

tColor const img[48] = {
        {2,2}, {2,2}, {2,1}, {1,1},
        {1,2}, {2,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,7},
        {1,1}, {1,1}, {3,4}, {5,6},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,1},
        {1,1}, {1,1}, {1,1}, {1,7},
        };

tSprite arrow = {
        .width = 4,
        .height = 12,
        .img = img
};

static void DispCursor(uint16_t x, uint16_t y) {
    static uint16_t ox, oy;

    if (x>0x7FFF) { // negative pos
        x = 0;
    }
    if (y>0x7FFF) { // negative pos
        y = 0;
    }
    if ((x+arrow.width)>=(ILI9341_WIDTH/2)) { // right border
        x = (ILI9341_WIDTH/2)-1-arrow.width;
    }
    if ((y+arrow.height)>=(ILI9341_HEIGHT)) { // bottom border
        y = ILI9341_HEIGHT-1-arrow.height;
    }
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
    static tColor bg[48];
    ClearDisplayWithBg();
    arrow.bg = bg;
    DispCursor(70, 150);
}
