#include <stdint.h>
#include "nd_usart.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

void usart_clock_setup(void)
{
	/* Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1. */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_USART1EN);
}

void usart_setup(void)
{
	/* Setup GPIO pin GPIO_USART1_TX/GPIO10 on GPIO port A for transmit. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 38400);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART1);
}

void usart_putc(char c) {
  usart_send_blocking(USART1, c);
}

uint8_t usart_getc() {
  return usart_recv(USART1);
}

//int puts(const char *s) {
//  //char c;
//  int i=0;
//  //c = s[0];
//  while(s[i] != 0) {
//    usart_putc(s[i++]);
 //   //i++;
 //   //c = s[i];
//  }
//  usart_putc('\n');
//  return i;
//}

void usart_puts(char *s) {
  char c;
  int i=0;
  c = s[0];
  while(c != 0) {
    usart_putc(c);
    i++;
    c = s[i];
  }
}

void usart_dec_u16(uint16_t c) {
  /* write the ascii, decimal representation of c to the UART */
  usart_putc('0' + (c / 10000));
  usart_putc('0' + ((c % 10000) / 1000));
  usart_putc('0' + ((c % 1000) / 100));
  usart_putc('0' + ((c % 100) / 10));
  usart_putc('0' + ((c % 10)));
}

void usart_dec_u32(uint32_t c) {
  /* write the ascii, decimal representation of c to the UART */
  usart_putc('0' + ((c / 1000000000)));
  usart_putc('0' + ((c % 100000000) / 10000000));
  usart_putc('0' + ((c % 10000000) / 1000000));
  usart_putc('0' + ((c % 1000000) / 100000));
  usart_putc('0' + ((c % 100000) / 10000));
  usart_putc('0' + ((c % 10000) / 1000));
  usart_putc('0' + ((c % 1000) / 100));
  usart_putc('0' + ((c % 100) / 10));
  usart_putc('0' + ((c % 10)));
}

void usart_hex_u8(uint8_t c) {
  if((c & 0xF0) > 0x90) {
    usart_putc(((c >> 4) & 0xF) + 'A' - 10);
  } else {
    usart_putc(((c >> 4) & 0xF) + '0');
  }
  if((c & 0xF) > 9) {
    usart_putc((c & 0xF) + 'A' - 10);
  } else {
    usart_putc((c & 0xF) + '0');
  }
}

