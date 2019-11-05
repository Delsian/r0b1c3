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

#define TOUCH_MEAS_INTERVAL     APP_TIMER_TICKS(500)

static nrf_saadc_value_t     m_buffer;
APP_TIMER_DEF(m_adc_timer_id);

static void touch_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

}

static void saadc_callback(const nrf_drv_saadc_evt_t * p_event)
{
    ret_code_t err_code;

    switch (p_event->type) {
    case NRF_DRV_SAADC_EVT_DONE:
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 1);
        APP_ERROR_CHECK(err_code);

        NRF_LOG_INFO("ADC %d", p_event->data.done.p_buffer[0]);
        break;
    case NRF_DRV_SAADC_EVT_CALIBRATEDONE:
        NRF_LOG_INFO("Calibrate");
        // Create timers.
        err_code = app_timer_create(&m_adc_timer_id,
                                    APP_TIMER_MODE_REPEATED,
                                    touch_timeout_handler);
        APP_ERROR_CHECK(err_code);    // Start application timers.
        err_code = app_timer_start(m_adc_timer_id, TOUCH_MEAS_INTERVAL, NULL);
        APP_ERROR_CHECK(err_code);
        break;
    default:
        break;
    }
}

void TouchInit(void) {
    ret_code_t err_code;

    nrf_gpio_cfg_output(PIN_TOUCHXL);
    nrf_gpio_pin_clear(PIN_TOUCHXL);
    nrf_gpio_cfg_output(PIN_TOUCHXR);
    nrf_gpio_pin_set(PIN_TOUCHXR);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_saadc_calibrate_offset();
    APP_ERROR_CHECK(err_code);
}

void TouchScan(void) {
    ret_code_t err_code;
    /* Init SAADC */
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(PINA_TOUCHYU);
    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&m_buffer, 1);
    APP_ERROR_CHECK(err_code);
}
