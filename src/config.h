#ifndef CONFIG_H
#define CONFIG_H 1

#include <libopencm3/stm32/usart.h>

/* Specify a USART device for each of std_out, _err and _in */
#define STD_OUT_UART USART1
#define STD_ERR_UART USART1
#define STD_IN_UART USART1

#define SUPPORT_FAT16 1
#define SUPPORT_FAT32 1

#endif

