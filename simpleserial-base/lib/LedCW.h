#ifndef LIB_LEDCW_H
#define LIB_LEDCW_H

#include <inttypes.h>

#include "hal.h"
#if HAL_TYPE == HAL_sam4s
#include "board.h"
#include "gpio.h"
#include "pio.h"
#elif HAL_TYPE == HAL_stm32f0 || HAL_TYPE == HAL_stm32f0_nano
#include "stm32f0xx_hal_rcc.h"
#include "stm32f0xx_hal_gpio.h"
#elif HAL_TYPE == HAL_stm32f1
#include "stm32f1xx_hal_gpio.h"
#elif HAL_TYPE == HAL_stm32f2
#include "stm32f2xx_hal_gpio.h"
#elif HAL_TYPE == HAL_stm32f3
#include "stm32f3xx_hal_rcc.h"
#include "stm32f3xx_hal_gpio.h"
#elif HAL_TYPE == HAL_stm32f4
#include "stm32f4xx_hal_gpio.h"
#elif HAL_TYPE == HAL_stm32l4
#include "stm32l4xx_hal_gpio.h"
#elif HAL_TYPE == HAL_stm32l5
#include "stm32l5xx_hal_gpio.h"
#endif

#if HAL_TYPE == HAL_sam4s
	static inline uint32_t led_gpio_id(uint8_t led){
		switch (led) {
		case 1:
			return PIO_PA16_IDX;
		case 2:
			return PIO_PA15_IDX;
		case 3:
			return PIO_PA14_IDX;
		default:
			return PIO_PA16_IDX;
		}
	}
#elif HAL_TYPE == HAL_stm32f0 || HAL_TYPE == HAL_stm32f1 || HAL_TYPE == HAL_stm32f2 || HAL_TYPE == HAL_stm32f4 || HAL_TYPE == HAL_stm32f0_nano || HAL_TYPE == HAL_stm32l4 || HAL_TYPE == HAL_stm32l5
#define LED_GPIO_PORT GPIOA
#define LED1_GPIO GPIO_PIN_2
#define LED2_GPIO GPIO_PIN_4
#define LED3_GPIO GPIO_PIN_5
#elif HAL_TYPE == HAL_stm32f3
#define LED_GPIO_PORT GPIOC
#define LED1_GPIO GPIO_PIN_13
#define LED2_GPIO GPIO_PIN_14
#define LED3_GPIO GPIO_PIN_15
#endif


/* ========================================================================== 
	LED control functions to turn on, turn off or toggle the leds
	available in the chipwhisperer boards for example, CWNANO has 
	two LEDs, one Red, one green this can be manipulated by the users,
	as indicators or flags that a function starts, end / finish, or 
	to have an indicator that a loop at least finishes or to visually
	observe the program behavior.
  =========================================================================== */

/*
 * @brief Initializes all supported LEDs as GPIO outputs and clears them.
 *
 * Configures the LED pins for the active HAL platform and turns
 * all LEDs off before use.
 */
void led_init(void);

/*
 * @brief Sets the hardware state of the specified LED.
 *
 * Updates the internal software state tracker and drives the
 * corresponding GPIO pin.
 *
 * @param led  LED identifier (1, 2, or 3).
 * @param on   Target state: non-zero to turn on, zero to turn off.
 */
void led_write(uint8_t led, uint8_t on);

/*
 * @brief Returns the last known software state of the specified LED.
 *
 * @param led  LED identifier (1, 2, or 3).
 * @return     Current state (0 = off, non-zero = on), or 0 if invalid.
 */
uint8_t led_state(uint8_t led);

/*
 * @brief SimpleSerial command handler to turn on an LED.
 *
 * Expects a one-byte payload containing the LED number.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  SimpleSerial subcommand byte.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on bad length or LED number.
 */
uint8_t led_on_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/*
 * @brief SimpleSerial command handler to turn off an LED.
 *
 * Expects a one-byte payload containing the LED number.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  SimpleSerial subcommand byte.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on bad length or LED number.
 */
uint8_t led_off_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/*
 * @brief SimpleSerial command handler to toggle an LED.
 *
 * Expects a one-byte payload containing the LED number.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  SimpleSerial subcommand byte.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on bad length or LED number.
 */
uint8_t led_toggle_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

#endif