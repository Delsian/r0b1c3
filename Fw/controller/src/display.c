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
#include "nrf_gfx.h"
#include "nrf_delay.h"
#include "display.h"

#define GRAY            0xC618
#define RED             0xF800
#define BLUE            0x001F
#define LINE_STEP       10

extern const nrf_gfx_font_desc_t orkney_24ptFontInfo;
extern const nrf_lcd_t nrf_lcd_ili9341;

static const nrf_gfx_font_desc_t * p_font = &orkney_24ptFontInfo;
static const nrf_lcd_t * p_lcd = &nrf_lcd_ili9341;

static void line_draw(void)
{
    nrf_gfx_line_t my_line = NRF_GFX_LINE(0, 0, 0, nrf_gfx_height_get(p_lcd), 2);
    nrf_gfx_line_t my_line_2 = NRF_GFX_LINE(nrf_gfx_width_get(p_lcd), nrf_gfx_height_get(p_lcd), 0, nrf_gfx_height_get(p_lcd), 1);

    for (uint16_t i = 0; i <= nrf_gfx_width_get(p_lcd); i += LINE_STEP)
    {
        my_line.x_end = i;
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &my_line, RED));
    }

    my_line.x_end = nrf_gfx_width_get(p_lcd);

    for (uint16_t i = 0; i <= nrf_gfx_height_get(p_lcd); i += LINE_STEP)
    {
        my_line.y_end = (nrf_gfx_height_get(p_lcd) - i);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &my_line, RED));
    }

    for (uint16_t i = 0; i <= nrf_gfx_height_get(p_lcd); i += LINE_STEP)
    {
        my_line_2.y_end = (nrf_gfx_height_get(p_lcd) - i);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &my_line_2, BLUE));
    }

    my_line_2.y_end = 0;

    for (uint16_t i = 0; i <= nrf_gfx_width_get(p_lcd); i += LINE_STEP)
    {
        my_line_2.x_end = i;
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &my_line_2, BLUE));
    }
}

void DisplayInit(void)
{
    nrf_gpio_cfg_output(ILI9341_RES_PIN);
    nrf_gpio_pin_set(ILI9341_RES_PIN);
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
}

void DispTest(void) {
    nrf_gfx_screen_fill(p_lcd, GRAY);
    nrf_delay_ms(200);
    //line_draw();
    DispCursor(150, 150);
}

void DispCursor(int x, int y) {
    nrf_gfx_line_t line1 = NRF_GFX_LINE(x-9, y, x+11, y, 3);
    nrf_gfx_line_t line2 = NRF_GFX_LINE(x, y-9, x, y+11, 3);
    nrf_gfx_line_draw(p_lcd, &line1, RED);
    nrf_gfx_line_draw(p_lcd, &line2, RED);

}
