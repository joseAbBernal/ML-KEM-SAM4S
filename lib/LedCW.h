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
	static uint32_t led_gpio_id(uint8_t led);
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
	two LEDs, one Red, one green this can be manipulated by the uses,
	as indicators or flags that a function stars, end / finish, or 
	to have an indicator that a loop at least finishes or to visually
	observe the program behavior.
  =========================================================================== */

/* Led initialization */
static void led_init(void);

/* Led On */
static uint8_t led_on_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/* Led Off */
static uint8_t led_off_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/* Led Toggle */
static uint8_t led_toggle_cmd(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);
