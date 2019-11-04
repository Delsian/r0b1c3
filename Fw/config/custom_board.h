
#ifndef CUSTBOARD_H
#define CUSTBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

#define LEDS_NUMBER    0
#define BUTTONS_NUMBER 0
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP


//=== Touchscreen inputs:
#define PIN_TOUCHXL         31
#define PIN_TOUCHYU         2
#define PIN_TOUCHXR         29
#define PIN_TOUCHYD         30

#define PINA_TOUCHXL			NRF_SAADC_INPUT_AIN7
#define PINA_TOUCHYU			NRF_SAADC_INPUT_AIN0
#define PINA_TOUCHXR            NRF_SAADC_INPUT_AIN5
#define PINA_TOUCHYD            NRF_SAADC_INPUT_AIN6

//===== Not used

#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 10
#define RTS_PIN_NUMBER 8
#define HWFC           true

#define SPIS_MISO_PIN  24    // SPI MISO signal.
#define SPIS_CSN_PIN   12    // SPI CSN signal.
#define SPIS_MOSI_PIN  25    // SPI MOSI signal.
#define SPIS_SCK_PIN   26    // SPI SCK signal.

#define SPIM0_SCK_PIN       4     /**< SPI clock GPIO pin number. */
#define SPIM0_MOSI_PIN      1     /**< SPI Master Out Slave In GPIO pin number. */
#define SPIM0_MISO_PIN      3     /**< SPI Master In Slave Out GPIO pin number. */
#define SPIM0_SS_PIN        2     /**< SPI Slave Select GPIO pin number. */

#define SPIM1_SCK_PIN       15     /**< SPI clock GPIO pin number. */
#define SPIM1_MOSI_PIN      12     /**< SPI Master Out Slave In GPIO pin number. */
#define SPIM1_MISO_PIN      14     /**< SPI Master In Slave Out GPIO pin number. */
#define SPIM1_SS_PIN        13     /**< SPI Slave Select GPIO pin number. */

// serialization APPLICATION board
#define SER_CONN_CHIP_RESET_PIN     12    // Pin used to reset connectivity chip

#define SER_APP_RX_PIN              25    // UART RX pin number.
#define SER_APP_TX_PIN              28    // UART TX pin number.
#define SER_APP_CTS_PIN             0     // UART Clear To Send pin number.
#define SER_APP_RTS_PIN             29    // UART Request To Send pin number.

#define SER_APP_SPIM0_SCK_PIN       7     // SPI clock GPIO pin number.
#define SER_APP_SPIM0_MOSI_PIN      0     // SPI Master Out Slave In GPIO pin number
#define SER_APP_SPIM0_MISO_PIN      30    // SPI Master In Slave Out GPIO pin number
#define SER_APP_SPIM0_SS_PIN        25    // SPI Slave Select GPIO pin number
#define SER_APP_SPIM0_RDY_PIN       29    // SPI READY GPIO pin number
#define SER_APP_SPIM0_REQ_PIN       28    // SPI REQUEST GPIO pin number

#ifdef __cplusplus
}
#endif

#endif // CUSTBOARD_H
