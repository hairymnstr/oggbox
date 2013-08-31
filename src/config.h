/*
 * config.h
 *
 * Contains all the hardware specific configuration and pin definitions
 */
#ifndef OGGBOX_H
#define OGGBOX_H 1

// SD card interface
#define SD_SPI        SPI2
#define SD_PORT       GPIOB
#define SD_CS_PIN     GPIO12
#define SD_MOSI_PIN   GPIO15
#define SD_MISO_PIN   GPIO14
#define SD_SCK_PIN    GPIO13
#define SD_WP_PIN     GPIO11
#define SD_WP_PORT    GPIOB
#define SD_CP_PIN     GPIO13
#define SD_CP_PORT    GPIOC

#define SD_SPI_APB    RCC_APB1ENR
#define SD_RCC_SPI    RCC_APB1ENR_SPI2EN
#define SD_IO_APB     RCC_APB2ENR
#define SD_RCC_IO     RCC_APB2ENR_IOPBEN

// LED indicator
#define GREEN_LED_PORT GPIOB
#define GREEN_LED_PIN GPIO1
#define RED_LED_PORT GPIOB
#define RED_LED_PIN GPIO0

// LCD pins
#define LCD_CS_PORT   GPIOB
#define LCD_CS_PIN    GPIO7
#define LCD_RST_PORT  GPIOB
#define LCD_RST_PIN   GPIO6
#define LCD_BL_PORT   GPIOB
#define LCD_BL_PIN    GPIO8
#define LCD_DC_PORT   GPIOB
#define LCD_DC_PIN    GPIO9
#define LCD_MOSI_PORT GPIOB
#define LCD_MOSI_PIN  GPIO5
#define LCD_SCK_PORT  GPIOB
#define LCD_SCK_PIN   GPIO3

#define LCD_SPI       SPI3

// codec hardware
#define CODEC_PORT      GPIOA
#define CODEC_CS        GPIO4
#define CODEC_SCK       GPIO5
#define CODEC_MISO      GPIO6
#define CODEC_MOSI      GPIO7

#define CODEC_DREQ_PORT GPIOA
#define CODEC_DREQ      GPIO8
#define CODEC_RST_PORT  GPIOA
#define CODEC_RST       GPIO3
#define CODEC_PWR_PORT  GPIOB
#define CODEC_PWR       GPIO5

#define CODEC_IOS_APB   RCC_APB2ENR
#define CODEC_RCC_IOS   RCC_APB2ENR_IOPAEN

#define CODEC_PWR_APB   RCC_APB2ENR
#define CODEC_RCC_PWR   RCC_APB2ENR_IOPBEN

#define CODEC_IOI_APB   RCC_APB2ENR
#define CODEC_RCC_IOI   RCC_APB2ENR_IOPCEN

#define CODEC_SPI       SPI1
#define CODEC_SPI_APB   RCC_APB2ENR
#define CODEC_RCC_SPI   RCC_APB2ENR_SPI1EN

// Power management
#define AUX_POWER_PORT GPIOC
#define AUX_POWER_PIN GPIO8

#define BAT_CHK_PORT  GPIOC
#define BAT_CHK_PIN   GPIO0

#define BAT_VAL_PORT  GPIOA
#define BAT_VAL_PIN   GPIO1

#define USB_P_PORT    GPIOC
#define USB_P_PIN     GPIO9

#define CHG_STAT_PORT GPIOC
#define CHG_STAT_PIN  GPIO10

#define CHG_100_PORT  GPIOC
#define CHG_100_PIN   GPIO11

#define CHG_500_PORT  GPIOC
#define CHG_500_PIN   GPIO12

// Button definitions
#define VOL_UP_PORT     GPIOC
#define VOL_UP_PIN      GPIO6

#define VOL_DOWN_PORT   GPIOC
#define VOL_DOWN_PIN    GPIO7

// USB stuff
#define USB_DISC_PORT   GPIOA
#define USB_DISC_PIN    GPIO2

// stdio definitions
#define STD_OUT_UART  USART1
#define STD_ERR_UART  USART1
#define STD_IN_UART   USART1

#endif /* ifndef OGGBOX_H */

