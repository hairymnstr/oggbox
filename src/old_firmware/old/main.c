/*
 * This file is part of the oggbox project.
 *
 * Copyright (C) 2010 Nathan Dumont (hairymnstr@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopenstm32/rcc.h>
#include <libopenstm32/gpio.h>
#include <libopenstm32/usart.h>
#include <libopenstm32/spi.h>     // debug, remove later

#include "ob_stdio.h"
#include "ob_screen.h"
#include "config.h"

extern void _etext, _data, _edata, _bss, _ebss, _stack;

void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* Enable GPIOC clock. */
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);
}

void gpio_setup(void)
{
	/* Set GPIO12 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
//  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
//          GPIO_CNF_OUTPUT_PUSHPULL, 0xFF);
//  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
//          GPIO_CNF_OUTPUT_PUSHPULL, GPIO8 | GPIO9 | GPIO10 | GPIO13);
//  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
//          GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
//  gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ,
//          GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
}

void usart_setup(void) {
  rcc_peripheral_enable_clock(&RCC_DBGURTIO, RCC_DBGURTIO_EN);
  rcc_peripheral_enable_clock(&RCC_DBGURT, RCC_DBGURT_EN);

  gpio_set_mode(DBG_URT_TX_BANK, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, DBG_URT_TX);

  usart_set_baudrate(DBG_URT, 38400);
  usart_set_databits(DBG_URT, 8);
  usart_set_stopbits(DBG_URT, USART_STOPBITS_1);
  usart_set_mode(DBG_URT, USART_MODE_TX);
  usart_set_parity(DBG_URT, USART_PARITY_NONE);
  usart_set_flow_control(DBG_URT, USART_FLOWCONTROL_NONE);

  usart_enable(DBG_URT);
}

int main(void) {
	int i, j;
  __builtin_memcpy(&_data, &_etext, &_edata - &_data);
  __builtin_memset(&_bss, 0, &_ebss - &_bss);

	clock_setup();
	gpio_setup();
	usart_setup();
  ob_screen_setup();

  ob_screen_set_bl(65535);

  ob_printf("Hello world the answer is %d\n", 42);

  unsigned short l = 0;
  ob_screen_set_bl(16384);

  ob_screen_set_contrast(32768);

  ob_screen_startup();

  ob_printf("running screen test\n");

//  ob_screen_test();

  ob_screen_print("Hello World");

  while(1) {;}
/*  gpio_set(SCREEN_RST_PORT, SCREEN_RST);

  gpio_clear(GPIOC, GPIO8);
  gpio_set(GPIOC, GPIO9);

  gpio_set(GPIOC, GPIO13);

  gpio_clear(GPIOA, GPIO1);
  gpio_set(GPIOD, GPIO2);

//  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0xAA;

//  ob_printf("%x, %x\n", GPIOC_CRL, GPIOC_CRH);
  //ob_printf("%x\n", 0x44444444);

//  ob_printf("Screen running\n");

  for(j=0;j<50;j++) {
    //ob_screen_set_bl(l);
    //l += 128;
  //  ob_printf("%d\n", l);
  //  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0xAA;
    for(i=0;i<1000000;i++);
    ob_printf("%x\n", SCREEN_DATA_IN);
  //  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0x55;
  //  for(i=0;i<1000000;i++);
  }

  gpio_clear(GPIOD, GPIO2);
  gpio_clear(GPIOC, GPIO13);
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
          GPIO_CNF_OUTPUT_PUSHPULL, 0xFF);

  ob_printf("%x, %x\n", GPIOC_CRL, GPIOC_CRH);

  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0x3F;
  for(j=0;j<50;j++) {
    //ob_screen_set_bl(l);
    //l += 128;
  //  ob_printf("%d\n", l);
  //  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0xAA;
    for(i=0;i<1000000;i++);
    ob_printf("%x\n", SCREEN_DATA_IN);
  //  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0x55;
  //  for(i=0;i<1000000;i++);
  }

  gpio_set(GPIOD, GPIO2);
  for(i=0;i<1000000;i++);
  gpio_clear(GPIOD, GPIO2);

  gpio_set_mode(GPIOC, GPIO_MODE_INPUT,
          GPIO_CNF_INPUT_FLOAT, 0xFF);

  for(i=0;i<1000000;i++);

  ob_printf("%x, %x\n", GPIOC_CRL, GPIOC_CRH);

  gpio_set(GPIOC, GPIO13);

  for(i=0;i<1000000;i++);

  gpio_set(GPIOD, GPIO2);

  for(j=0;j<50;j++) {
    //ob_screen_set_bl(l);
    //l += 128;
  //  ob_printf("%d\n", l);
  //  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0xAA;
    for(i=0;i<1000000;i++);
    ob_printf("%x\n", SCREEN_DATA_IN);
  //  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0x55;
  //  for(i=0;i<1000000;i++);
  }
 
  gpio_clear(GPIOD, GPIO2);

  for(i=0;i<1000000;i++);

  gpio_set(GPIOD, GPIO2);

  for(j=0;j<50;j++) {
    //ob_screen_set_bl(l);
    //l += 128;
  //  ob_printf("%d\n", l);
  //  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0xAA;
    for(i=0;i<1000000;i++);
    ob_printf("%x\n", SCREEN_DATA_IN);
  //  SCREEN_DATA_OUT = (SCREEN_DATA_OUT & 0xFF00) | 0x55;
  //  for(i=0;i<1000000;i++);
  }
 */

	return 0;
}

