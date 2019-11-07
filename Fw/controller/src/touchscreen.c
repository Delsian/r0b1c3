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
#include "nrf_drv_saadc.h"
#include "nrf_log.h"
#include "app_timer.h"

#define TOUCH_MEAS_INTERVAL     APP_TIMER_TICKS(100)
#define DEF_DX 150
#define DEF_MX 256
#define DEF_DY 150
#define DEF_MY 256

static nrf_saadc_value_t     m_buffer[2];
APP_TIMER_DEF(m_adc_timer_id);

typedef enum {
    READSTATE_X,
    READSTATE_Y
} eReadState;

static eReadState rs = READSTATE_X;
static nrf_saadc_channel_config_t channel_config[2] = {
    NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(PINA_TOUCHXL),
    NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(PINA_TOUCHYD),
};

// stored value
static int16_t coord_x, coord_y;
// Calibration
static int16_t dx, mx, dy, my;
static int16_t report[2];

static void set_pin_x(void) {
    nrf_gpio_pin_clear(PIN_TOUCHYD); // discharge

    nrf_gpio_cfg(
            PIN_TOUCHXL,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_H0D1,
            NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(
            PIN_TOUCHXR,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_H0H1,
            NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(PIN_TOUCHXL);
    nrf_gpio_pin_set(PIN_TOUCHXR);

    nrf_gpio_cfg_input(PIN_TOUCHYU, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_input(PIN_TOUCHYD, NRF_GPIO_PIN_PULLDOWN);
}

static void set_pin_y(void) {
    nrf_gpio_pin_clear(PIN_TOUCHXR); // discharge

    nrf_gpio_cfg(
            PIN_TOUCHYU,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_H0D1,
            NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(
            PIN_TOUCHYD,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_H0H1,
            NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(PIN_TOUCHYU);
    nrf_gpio_pin_set(PIN_TOUCHYD);

    nrf_gpio_cfg_input(PIN_TOUCHXR, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_input(PIN_TOUCHXL, NRF_GPIO_PIN_PULLDOWN);
}

static void touch_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    APP_ERROR_CHECK(nrf_drv_saadc_sample());
}

static void timer_init(void) {
    set_pin_x();
    // Create timers.
    APP_ERROR_CHECK(app_timer_create(&m_adc_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                touch_timeout_handler));
    APP_ERROR_CHECK(app_timer_start(m_adc_timer_id, TOUCH_MEAS_INTERVAL, NULL));
}

static void saadc_callback(const nrf_drv_saadc_evt_t * p_event)
{
    const ControlEvent cevt  = {
            .type = CE_TOUCH,
            .ptr16 = report
    };
    switch (p_event->type) {
    case NRF_DRV_SAADC_EVT_DONE:
        switch(rs) {
        case READSTATE_X:
            coord_x = p_event->data.done.p_buffer[1];
            set_pin_y();
            rs = READSTATE_Y;
            APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 2));
            APP_ERROR_CHECK(nrf_drv_saadc_sample());
            break;
        case READSTATE_Y:
            coord_y = p_event->data.done.p_buffer[0];
            set_pin_x();
            rs = READSTATE_X;
            APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 2));

            // Report
            if (coord_x > 80 && coord_y > 80) {
                NRF_LOG_INFO("Coord %d %d", coord_x, coord_y);
                // todo: Convert

                report[0] = ((coord_x - dx) * mx) >> 8;
                report[1] = ((coord_y - dy) * my) >> 8;
                ControlPost(&cevt);
            }
            break;
        default:
            break;
        }
        break;
    case NRF_DRV_SAADC_EVT_CALIBRATEDONE:
        NRF_LOG_INFO("Calibrate");
        // Configure channels
        APP_ERROR_CHECK(nrf_drv_saadc_channel_init(0, &(channel_config[0])));
        APP_ERROR_CHECK(nrf_drv_saadc_channel_init(1, &(channel_config[1])));
        APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(m_buffer, 2));
        timer_init();
        break;
    default:
        break;
    }
}

void TouchInit(void) {
    ret_code_t err_code;

    channel_config[0].acq_time = NRF_SAADC_ACQTIME_3US;
    channel_config[1].acq_time = NRF_SAADC_ACQTIME_3US;
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_saadc_calibrate_offset();
    APP_ERROR_CHECK(err_code);

    // Set defaults
    dx = DEF_DX;
    mx = DEF_MX;
    dy = DEF_DY;
    my = DEF_MY;
}

void TouchCalibrate(int16_t idx, int16_t imx, int16_t idy, int16_t imy) {
    dx = idx;
    mx = imx;
    dy = idy;
    my = imy;
}
