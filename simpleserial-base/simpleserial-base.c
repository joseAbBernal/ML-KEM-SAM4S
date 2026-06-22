/*
This file is part of the ChipWhisperer Example Targets
Copyright (C) 2012-2017 NewAE Technology Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "simpleserial.h"

#if HAL_TYPE == HAL_stm32f0_nano
#define SK_LEN 800
#define PK_LEN 800
#define PT_LEN 100
#define CT_LEN 768
#define SS_LEN 32
#else
#define SK_LEN 1632
#define PK_LEN 800
#define PT_LEN 768
#define CT_LEN 768
#define SS_LEN 32
#endif

uint16_t CHUNK_SZ = 248; // Tamaño de chunk por defecto (1-248)

#if HAL_TYPE == HAL_stm32f0_nano
static uint8_t sk[SK_LEN] = {0};
static uint8_t pk[PK_LEN] = {0};
static uint8_t pt[100] = {0};
static uint8_t ct[CT_LEN] = {0};
static uint8_t ss[SS_LEN] = {0};
#else
static uint8_t sk[SK_LEN] = {0};
static uint8_t pk[PK_LEN] = {0};
static uint8_t pt[PT_LEN] = {0};
static uint8_t ct[CT_LEN] = {0};
static uint8_t ss[SS_LEN] = {0};
#endif

#if HAL_TYPE == HAL_sam4s
static uint32_t led_gpio_id(uint8_t led)
{
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

static volatile uint8_t LED1State = 0;
static volatile uint8_t LED2State = 0;
static volatile uint8_t LED3State = 0;

static void led_write(uint8_t led, uint8_t on)
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

static uint8_t led_state(uint8_t led)
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

static void led_init(void)
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

static uint8_t led_on_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
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

static uint8_t led_off_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
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

static uint8_t led_toggle_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
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

#if SS_VER != SS_VER_2_1
static uint8_t led_on_cmd_v11(uint8_t *buf, uint8_t len)
{
	return led_on_cmd(0, 0, len, buf);
}

static uint8_t led_off_cmd_v11(uint8_t *buf, uint8_t len)
{
	return led_off_cmd(0, 0, len, buf);
}

static uint8_t led_toggle_cmd_v11(uint8_t *buf, uint8_t len)
{
	return led_toggle_cmd(0, 0, len, buf);
}
#endif

uint8_t get_key(uint8_t* k, uint8_t len)
{
	// Load key here
	return 0x00;
}

uint8_t get_pt(uint8_t* pt, uint8_t len)
{
	/**********************************
	* Start user-specific code here. */
	trigger_high();

	//16 hex bytes held in 'pt' were sent
	//from the computer. Store your response
	//back into 'pt', which will send 16 bytes
	//back to computer. Can ignore of course if
	//not needed

	trigger_low();
	/* End user-specific code here. *
	********************************/
	simpleserial_put('r', 16, pt);
	return 0x00;
}

uint8_t reset(uint8_t* x, uint8_t len)
{
	// Reset key here if needed
	return 0x00;
}

uint8_t rx_data(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint8_t *dst = NULL;
	uint16_t dst_len = 0;
	uint16_t offset;
	uint16_t to_copy;

	(void)cmd;

	switch (scmd & 0xF0) {
	case 0xB0: // SK
		dst = sk;
		dst_len = SK_LEN;
		break;
	case 0xC0: // PK
		dst = pk;
		dst_len = PK_LEN;
		break;
	case 0xD0: // CT
		dst = ct;
		dst_len = CT_LEN;
		break;
	case 0xE0: // PT
		dst = pt;
		dst_len = PT_LEN;
		break;
	case 0xF0: // SS
		dst = ss;
		dst_len = SS_LEN;
		break;
	default:
		return SS_ERR_CMD;
	}

	offset = (uint16_t)(scmd & 0x0F) * CHUNK_SZ;
	if (offset >= dst_len) {
		return SS_ERR_LEN;
	}

	if (len == 0 || len > CHUNK_SZ) {
		return SS_ERR_LEN;
	}

	if (offset + len > dst_len) {
		return SS_ERR_LEN;
	}

	to_copy = len;
	memcpy(dst + offset, buf, to_copy);
	return SS_ERR_OK;
}

uint8_t rx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validar que no sobrepasamos los límites
	if (offset >= SK_LEN) return SS_ERR_LEN;
	// Limitar la cantidad a copiar al espacio disponible
	uint16_t to_copy = (offset + len > SK_LEN) ? SK_LEN - offset : len;
	memcpy(sk + offset, buf, to_copy);
	return 0x00;
}

uint8_t tx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validar que el offset está dentro de límites
	if (offset >= SK_LEN) return SS_ERR_LEN;
	// Enviar el chunk de datos (CHUNK_SZ o menos si llegamos al final)
	uint16_t to_send = (offset + CHUNK_SZ > SK_LEN) ? SK_LEN - offset : CHUNK_SZ;
	simpleserial_put('k', to_send, sk + offset);
	return 0x00;
}

uint8_t rx_pk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	if (offset >= PK_LEN) return SS_ERR_LEN;
	uint16_t to_copy = (offset + len > PK_LEN) ? PK_LEN - offset : len;
	memcpy(pk + offset, buf, to_copy);
	return 0x00;
}

uint8_t tx_pk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	if (offset >= PK_LEN) return SS_ERR_LEN;
	uint16_t to_send = (offset + CHUNK_SZ > PK_LEN) ? PK_LEN - offset : CHUNK_SZ;
	simpleserial_put('p', to_send, pk + offset);
	return 0x00;
}

uint8_t rx_ct(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	if (offset >= CT_LEN) return SS_ERR_LEN;
	uint16_t to_copy = (offset + len > CT_LEN) ? CT_LEN - offset : len;
	memcpy(ct + offset, buf, to_copy);
	return 0x00;
}

uint8_t tx_ct(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	if (offset >= CT_LEN) return SS_ERR_LEN;
	uint16_t to_send = (offset + CHUNK_SZ > CT_LEN) ? CT_LEN - offset : CHUNK_SZ;
	simpleserial_put('c', to_send, ct + offset);
	return 0x00;
}

uint8_t rx_ss(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	if (offset >= SS_LEN) return SS_ERR_LEN;
	uint16_t to_copy = (offset + len > SS_LEN) ? SS_LEN - offset : len;
	memcpy(ss + offset, buf, to_copy);
	return 0x00;
}

uint8_t tx_ss(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	if (offset >= SS_LEN) return SS_ERR_LEN;
	uint16_t to_send = (offset + CHUNK_SZ > SS_LEN) ? SS_LEN - offset : CHUNK_SZ;
	simpleserial_put('s', to_send, ss + offset);
	return 0x00;
}

uint8_t set_chunk_size(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	if (len != 2) return SS_ERR_LEN; // Esperamos exactamente 2 bytes (uint16_t)
	uint16_t new_size = buf[0] | (buf[1] << 8);
	if (new_size < 1 || new_size > 248) return SS_ERR_LEN; // Límite de SimpleSerial v2.1
	CHUNK_SZ = new_size;
	return 0x00;
}

#if SS_VER == SS_VER_2_1
uint8_t aes(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint8_t req_len = 0;
	uint8_t err = 0;

	if (scmd & 0x02) {
		req_len += 16;
		if (req_len > len) {
			return SS_ERR_LEN;
		}
		err = get_key(buf + req_len - 16, 16);
		if (err)
			return err;
	}
	if (scmd & 0x01) {
		req_len += 16;
		if (req_len > len) {
			return SS_ERR_LEN;
		}
		err = get_pt(buf + req_len - 16, 16);
		if (err)
			return err;
	}

	if (len != req_len) {
		return SS_ERR_LEN;
	}

	return 0x00;

}
#endif

int main(void)
{
	platform_init();
	led_init();
	init_uart();
	trigger_setup();

/* Uncomment this to get a HELLO message for debug */
/*
putch('h');
putch('e');
putch('l');
putch('l');
putch('o');
putch('\n');
*/

    simpleserial_init();
#if SS_VER != SS_VER_2_1
	simpleserial_addcmd('p', 16, get_pt);
	simpleserial_addcmd('k', 16, get_key);
	simpleserial_addcmd('x', 0, reset);
	simpleserial_addcmd('h', 1, led_on_cmd_v11);
	simpleserial_addcmd('l', 1, led_off_cmd_v11);
	simpleserial_addcmd('t', 1, led_toggle_cmd_v11);
#else
	simpleserial_addcmd(0x01, 16, aes);
	simpleserial_addcmd(0x02, 248, rx_key); // SK: receive chunk
	simpleserial_addcmd(0x03, 0, tx_key); // SK: send chunk
	simpleserial_addcmd(0x04, 2, set_chunk_size); // set chunk size
	simpleserial_addcmd(0x05, 248, rx_pk); // PK: receive chunk
	simpleserial_addcmd(0x06, 0, tx_pk); // PK: send chunk
	simpleserial_addcmd(0x07, 248, rx_ct); // CT: receive chunk
	simpleserial_addcmd(0x08, 0, tx_ct); // CT: send chunk
	simpleserial_addcmd(0x09, 248, rx_ss); // SS: receive chunk
	simpleserial_addcmd(0x0A, 0, tx_ss); // SS: send chunk
	simpleserial_addcmd(0x0B, 248, rx_data); // Receive data SK, PK, CT, PT, SS.
	simpleserial_addcmd('h', 1, led_on_cmd);
	simpleserial_addcmd('l', 1, led_off_cmd);
	simpleserial_addcmd('t', 1, led_toggle_cmd);

#endif
	while(1)
		simpleserial_get();
}