#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "simpleserial.h"
#include "LedCW.h"

static volatile uint8_t LED1State = 0;
static volatile uint8_t LED2State = 0;
static volatile uint8_t LED3State = 0;

/*
 * @brief Sets the hardware state of the specified LED.
 *
 * Updates the internal LED state tracking variables and writes
 * the corresponding hardware register depending on the active
 * HAL platform.
 *
 * @param led  LED identifier (1, 2, or 3).
 * @param on   Target state: non-zero to turn on, zero to turn off.
 */
void led_write(uint8_t led, uint8_t on)
{
	switch (led) {
	case 1:
		LED1State = on;
		break;
	case 2:
		LED2State = on;
		break;
	case 3:
		LED3State = on;
		break;
	default:
		return;
	}

#if HAL_TYPE == HAL_sam4s
	uint32_t gpio = led_gpio_id(led);
	if (on) {
		gpio_set_pin_high(gpio);
	} else {
		gpio_set_pin_low(gpio);
	}
#elif HAL_TYPE == HAL_stm32f0 || HAL_TYPE == HAL_stm32f1 || HAL_TYPE == HAL_stm32f2 || HAL_TYPE == HAL_stm32f3 || HAL_TYPE == HAL_stm32f4 || HAL_TYPE == HAL_stm32f0_nano || HAL_TYPE == HAL_stm32l4 || HAL_TYPE == HAL_stm32l5
	switch (led) {
	case 1:
		HAL_GPIO_WritePin(LED_GPIO_PORT, LED1_GPIO, on ? SET : RESET);
		break;
	case 2:
		HAL_GPIO_WritePin(LED_GPIO_PORT, LED2_GPIO, on ? SET : RESET);
		break;
	case 3:
		HAL_GPIO_WritePin(LED_GPIO_PORT, LED3_GPIO, on ? SET : RESET);
		break;
	default:
		break;
	}
#else
	(void)on;
#endif
}

/*
 * @brief Returns the last known software state of the specified LED.
 *
 * @param led  LED identifier (1, 2, or 3).
 * @return     Current state of the LED (0 = off, non-zero = on),
 *             or 0 if the LED identifier is invalid.
 */
uint8_t led_state(uint8_t led)
{
	switch (led) {
	case 1:
		return LED1State;
	case 2:
		return LED2State;
	case 3:
		return LED3State;
	default:
		return 0;
	}
}

/*
 * @brief Initializes all supported LEDs as outputs and clears them.
 *
 * Configures the GPIO pins for the available LEDs according to the
 * selected HAL target and ensures all LEDs start in the off state.
 */
void led_init(void)
{
#if HAL_TYPE == HAL_sam4s
	gpio_configure_pin(led_gpio_id(1), PIO_OUTPUT_0 | PIO_DEFAULT);
	gpio_configure_pin(led_gpio_id(2), PIO_OUTPUT_0 | PIO_DEFAULT);
	gpio_configure_pin(led_gpio_id(3), PIO_OUTPUT_0 | PIO_DEFAULT);
#elif HAL_TYPE == HAL_stm32f0 || HAL_TYPE == HAL_stm32f1 || HAL_TYPE == HAL_stm32f2 || HAL_TYPE == HAL_stm32f3 || HAL_TYPE == HAL_stm32f4 || HAL_TYPE == HAL_stm32f0_nano || HAL_TYPE == HAL_stm32l4 || HAL_TYPE == HAL_stm32l5
    GPIO_InitTypeDef GpioInit;
	GpioInit.Pin = LED1_GPIO | LED2_GPIO | LED3_GPIO;
	GpioInit.Mode = GPIO_MODE_OUTPUT_PP;
	GpioInit.Pull = GPIO_NOPULL;
	GpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
#if HAL_TYPE == HAL_stm32f3
	__HAL_RCC_GPIOC_CLK_ENABLE();
#else
	__HAL_RCC_GPIOA_CLK_ENABLE();
#endif
	HAL_GPIO_Init(LED_GPIO_PORT, &GpioInit);
#endif

	led_write(1, 0);
	led_write(2, 0);
	led_write(3, 0);
}

/*
 * @brief SimpleSerial command handler to turn on an LED.
 *
 * Expects a single-byte payload indicating the LED number.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  SimpleSerial subcommand byte.
 * @param len   Length of the payload in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on invalid length or
 *              invalid LED number.
 */
uint8_t led_on_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	(void)cmd;
	(void)scmd;

	if (len != 1) {
		return SS_ERR_LEN;
	}

	if (buf[0] < 1 || buf[0] > 3) {
		return SS_ERR_LEN;
	}

	led_write(buf[0], 1);
	return SS_ERR_OK;
}

/*
 * @brief SimpleSerial command handler to turn off an LED.
 *
 * Expects a single-byte payload indicating the LED number.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  SimpleSerial subcommand byte.
 * @param len   Length of the payload in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on invalid length or
 *              invalid LED number.
 */
uint8_t led_off_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	(void)cmd;
	(void)scmd;

	if (len != 1) {
		return SS_ERR_LEN;
	}

	if (buf[0] < 1 || buf[0] > 3) {
		return SS_ERR_LEN;
	}

	led_write(buf[0], 0);
	return SS_ERR_OK;
}

/*
 * @brief SimpleSerial command handler to toggle an LED.
 *
 * Expects a single-byte payload indicating the LED number.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  SimpleSerial subcommand byte.
 * @param len   Length of the payload in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on invalid length or
 *              invalid LED number.
 */
uint8_t led_toggle_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t state;

	(void)cmd;
	(void)scmd;

	if (len != 1) {
		return SS_ERR_LEN;
	}

	state = led_state(buf[0]);
	led_write(buf[0], state ? 0 : 1);
	return SS_ERR_OK;
}
