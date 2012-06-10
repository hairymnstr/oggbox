#ifndef OGGBOX_H
#define OGGBOX_H 1

#include <libopencm3/stm32/f1/gpio.h>

#define GREEN_LED_PORT GPIOB
#define GREEN_LED_PIN GPIO1
#define RED_LED_PORT GPIOB
#define RED_LED_PIN GPIO0

#define LCD_E_PORT GPIOC
#define LCD_E_PIN GPIO10
#define LCD_CS_PORT GPIOC
#define LCD_CS_PIN GPIO9
#define LCD_RW_PORT GPIOC
#define LCD_RW_PIN GPIO11
#define LCD_RST_PORT GPIOC
#define LCD_RST_PIN GPIO8
#define LCD_DC_PORT GPIOA
#define LCD_DC_PIN GPIO8
#define LCD_BL_PORT GPIOA
#define LCD_BL_PIN GPIO2

#define LCD_DATA_PORT GPIOC
#define LCD_DATA_MASK 0xFF

#endif /* ifndef OGGBOX_H */
