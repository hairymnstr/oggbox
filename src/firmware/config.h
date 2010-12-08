#ifndef CONFIG_H
#define CONFIG_H

#include <libopenstm32/usart.h>
#include <libopenstm32/gpio.h>
#include <libopenstm32/usart.h>

/* debug uart defines */
#define RCC_DBGURTIO      RCC_APB2ENR
#define RCC_DBGURTIO_EN   RCC_APB2ENR_IOPAEN
#define RCC_DBGURT        RCC_APB1ENR
#define RCC_DBGURT_EN     RCC_APB1ENR_USART2EN
#define DBG_URT_TX_BANK   GPIOA
#define DBG_URT_TX        GPIO_USART2_TX
#define DBG_URT           USART2

/* screen pins */
#define RCC_SCREEN_BL     RCC_APB2ENR
#define RCC_SCREEN_BL_EN  RCC_APB2ENR_IOPAEN
#define SCREEN_BL_PORT    GPIOA
#define SCREEN_BL         GPIO9
#define SCREEN_CON_PORT   GPIOA
#define SCREEN_CON        GPIO10

#define SCREEN_DATA_PORT  GPIOC
#define SCREEN_DATA_IN    GPIOC_IDR
#define SCREEN_DATA_OUT   GPIOC_ODR

#define SCREEN_CS1_PORT   GPIOC
#define SCREEN_CS1        GPIO8
#define SCREEN_CS2_PORT   GPIOC
#define SCREEN_CS2        GPIO9
#define SCREEN_RST_PORT   GPIOC
#define SCREEN_RST        GPIO10
#define SCREEN_RW_PORT    GPIOC
#define SCREEN_RW         GPIO13
#define SCREEN_E_PORT     GPIOD
#define SCREEN_E          GPIO2
#define SCREEN_DI_PORT    GPIOA
#define SCREEN_DI         GPIO1


/* screen messages */
#define SCREEN_ON         0x3F
#define SCREEN_ROW        0xB8
#define SCREEN_COL        0x40

#endif

