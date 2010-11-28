#ifndef CONFIG_H
#define CONFIG_H

#include <libopenstm32/usart.h>
#include <libopenstm32/gpio.h>
#include <libopenstm32/usart.h>

#define RCC_DBGURTIO      RCC_APB2ENR
#define RCC_DBGURTIO_EN   RCC_APB2ENR_IOPAEN
#define RCC_DBGURT        RCC_APB1ENR
#define RCC_DBGURT_EN     RCC_APB1ENR_USART2EN
#define DBG_URT_TX_BANK   GPIOA
#define DBG_URT_TX        GPIO_USART2_TX
#define DBG_URT           USART2

#endif

