#ifndef OGGBOX_H
#define OGGBOX_H 1

#include <libopencm3/stm32/f1/gpio.h>

#define SD_SPI SPI2
#define SD_PORT GPIOB
#define SD_CS GPIO12
#define SD_MOSI GPIO15
#define SD_MISO GPIO14
#define SD_SCK GPIO13
#define SD_WP GPIO11
#define SD_WP_PORT GPIOB
#define SD_CP GPIO13
#define SD_CP_PORT GPIOC

#define SD_SPI_APB RCC_APB1ENR
#define SD_RCC_SPI RCC_APB1ENR_SPI2EN
#define SD_IO_APB RCC_APB2ENR
#define SD_RCC_IO RCC_APB2ENR_IOPBEN

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

#define AUX_POWER_PORT GPIOC
#define AUX_POWER_PIN GPIO8

#endif /* ifndef OGGBOX_H */