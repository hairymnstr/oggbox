#ifndef ND_USART_H
#define ND_USART_H 1

#include <stdint.h>

void usart_clock_setup(void);
void usart_setup(void);
void usart_putc(char);
uint8_t usart_getc(void);
void usart_puts(char *);
void usart_dec_u16(uint16_t);
void usart_dec_u32(uint32_t);
void usart_hex_u8(uint8_t);
int puts(const char *);

#endif

