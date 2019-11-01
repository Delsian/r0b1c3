
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bluetooth.h"

#include "nrf_pwr_mgmt.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "nrf_soc.h"
#include "nrf_clock.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// Components
#include "boards.h"
#include "control.h"
#include "custom_service.h"
//#include "localfds.h"

static void clock_irq_handler(nrfx_clock_evt_type_t evt)
{

}

static void HwInit(void)
{
	ret_code_t err_code;

	/* Enable power switch */
	nrf_gpio_cfg_output(22);
	nrf_gpio_pin_set(22);

    /* initializing the Power manager. */
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_clock_init(clock_irq_handler);
    APP_ERROR_CHECK(err_code);

    // Initialize timer module, making it use the scheduler
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

int main(void)
{
    (void) NRF_LOG_INIT(NULL);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("main()");

    HwInit();
    ControlInit();
    ble_stack_init();

	while (1)
	{
		app_sched_execute();
		(void)sd_app_evt_wait();
	}
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}
