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

uint16_t img[64] = {
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456,
                0x3456, 0x3456, 0x3456, 0x3456
        };

const tSprite arrow = {
        .width = 8,
        .height = 8,
        .img = img
};

static void DispCursor(uint16_t x, uint16_t y) {
    static uint16_t ox, oy;
    ili9341_rect_draw( ox, oy, 8, 8, GRAY);
    ili9341_sprite_draw( x, y, &arrow);
    ox = x;
    oy = y;
}

static void DispTouch(const ControlEvent* pEvt) {
    uint16_t *pos = pEvt->ptr16;
    NRF_LOG_INFO("Disp %d %d", pos[0], pos[1]);
    DispCursor(pos[0], pos[1]);
}

void DisplayInit(void)
{
    nrf_gpio_cfg_output(ILI9341_RES_PIN);
    nrf_gpio_pin_set(ILI9341_RES_PIN);
    APP_ERROR_CHECK(ili9341_init());
    ControlRegisterCb(CE_TOUCH, DispTouch);
}

void DispTest(void) {
    ili9341_rect_draw( 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, GRAY);
    nrf_delay_ms(200);
    //line_draw();
    DispCursor(150, 150);
}
