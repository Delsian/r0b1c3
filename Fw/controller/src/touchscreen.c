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

static nrf_saadc_value_t     m_buffer[2];
APP_TIMER_DEF(m_adc_timer_id);

typedef enum {
    READSTATE_X,
    READSTATE_Y,
    SETUP_X,
    SETUP_Y
} eReadState;

static eReadState rs = READSTATE_X;
static nrf_saadc_channel_config_t channel_config =
    NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(PINA_TOUCHXL);

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

    channel_config.pin_p = (nrf_saadc_input_t)(PINA_TOUCHYU);
    APP_ERROR_CHECK(nrf_drv_saadc_channel_init(0, &channel_config));
    APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(&(m_buffer[0]), 1));
}

static void set_pin_y(void) {
    APP_ERROR_CHECK(nrf_drv_saadc_channel_uninit(0));

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

    channel_config.pin_p = (nrf_saadc_input_t)(PINA_TOUCHXL);
    APP_ERROR_CHECK(nrf_drv_saadc_channel_init(0, &channel_config));
    APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(&(m_buffer[1]), 1));
}

static void touch_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    switch(rs) {
    case READSTATE_X:
        APP_ERROR_CHECK(nrf_drv_saadc_sample());
        rs = SETUP_Y;
        break;
    case READSTATE_Y:
        APP_ERROR_CHECK(nrf_drv_saadc_sample());
        rs = SETUP_X;
        break;
    case SETUP_X:
        NRF_LOG_INFO("Coord %d %d", m_buffer[0], m_buffer[1]);
        APP_ERROR_CHECK(nrf_drv_saadc_channel_uninit(0)); // no first init
        set_pin_x();
        rs = READSTATE_X;
        break;
    case SETUP_Y:
        set_pin_y();
        rs = READSTATE_Y;
        break;
    }
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
    switch (p_event->type) {
    case NRF_DRV_SAADC_EVT_DONE:
        //NRF_LOG_INFO("ADC%d %d", rs, p_event->data.done.p_buffer[0]);
        break;
    case NRF_DRV_SAADC_EVT_CALIBRATEDONE:
        NRF_LOG_INFO("Calibrate");
        timer_init();
        break;
    default:
        break;
    }
}

void TouchInit(void) {
    ret_code_t err_code;

    channel_config.acq_time = NRF_SAADC_ACQTIME_3US;
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);
    //timer_init();
    err_code = nrf_drv_saadc_calibrate_offset();
    APP_ERROR_CHECK(err_code);
}

