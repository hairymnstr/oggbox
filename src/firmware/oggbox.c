/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

// based on miniblink from the examples
// OggBox has green of bi-color led on PB1 so setup GPIOB for output

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>

#include <stdio.h>

#include "dirent.h"

#include "oggbox.h"
#include "lcd.h"
#include "ui.h"
#include "sdfat.h"
#include "nd_usart.h"

extern void _etext, _data, _edata, _bss, _ebss, _stack;

void gpio_setup(void)
{
        rcc_clock_setup_in_hse_8mhz_out_72mhz();
	/* Enable GPIOC clock. */
	/* Manually: */
	// RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;
	/* Using API functions: */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
        rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
        rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);

	/* Set GPIO12 (in GPIO port C) to 'output push-pull'. */
	/* Manually: */
	// GPIOC_CRH = (GPIO_CNF_OUTPUT_PUSHPULL << (((12 - 8) * 4) + 2));
	// GPIOC_CRH |= (GPIO_MODE_OUTPUT_2_MHZ << ((12 - 8) * 4));
	/* Using API functions: */
	gpio_set_mode(GREEN_LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, RED_LED_PIN | GREEN_LED_PIN);
}

int main(void)
{
	int i;
//         DIR *dr;
//         struct dirent *de;

//         __builtin_memcpy(&_data, &_etext, &_edata - &_data);
//         __builtin_memset(&_bss, 0, &_ebss - &_bss);

        
        usart_clock_setup();
	gpio_setup();
        usart_setup();
        lcdInit();
        
        lcdClear();
        
        lcdBacklight(32768);
        
//         sdfat_init();
//         sdfat_mount();
        
        usart_puts("Hello World\r\n");
        _write_r(NULL, STDOUT_FILENO, "Hello Worl2\r\n", 13);
        fwrite("Hello Worl3\r\n", 1, 13, stdout);
        
        uiShowSD(gpio_port_read(GPIOD) & 4);    // SD absent
        
        lcdPrintPortrait(" OggBox", 2);
        lcdPrintPortrait("  RevA", 3);
//     dr = opendir("/");
//     de = readdir(dr);
    
//         lcdPrintPortrait(de->d_name, 5);
        
        gpio_set(RED_LED_PORT, RED_LED_PIN);

	/* Blink the LED (PC12) on the board. */
	while (1) {
		/* Manually: */
		// GPIOC_BSRR = GPIO12;		/* LED off */
		// for (i = 0; i < 800000; i++)	/* Wait a bit. */
		// 	__asm__("nop");
		// GPIOC_BRR = GPIO12;		/* LED on */
		// for (i = 0; i < 800000; i++)	/* Wait a bit. */
		// 	__asm__("nop");

		/* Using API functions gpio_set()/gpio_clear(): */
		// gpio_set(GPIOC, GPIO12);	/* LED off */
		// for (i = 0; i < 800000; i++)	/* Wait a bit. */
		// 	__asm__("nop");
		// gpio_clear(GPIOC, GPIO12);	/* LED on */
		// for (i = 0; i < 800000; i++)	/* Wait a bit. */
		// 	__asm__("nop");

		/* Using API function gpio_toggle(): */
		gpio_toggle(GREEN_LED_PORT, GREEN_LED_PIN);	/* LED on/off */
		for (i = 0; i < 10000; i++)    {	/* Wait a bit. */
			if(gpio_port_read(GPIOD) & 4)
                          gpio_set(RED_LED_PORT, RED_LED_PIN);
                        else
                          gpio_clear(RED_LED_PORT, RED_LED_PIN);
                        
                        uiShowSD(gpio_port_read(GPIOC) & (1 << 13));
                        uiShowLocked(gpio_port_read(GPIOD) & 4);
                }
	}

	return 0;
}
