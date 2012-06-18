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

#include "oggbox.h"
#include "lcd.h"

void gpio_setup(void)
{
	/* Enable GPIOC clock. */
	/* Manually: */
	// RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;
	/* Using API functions: */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
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

	gpio_setup();
        lcdInit();
        
        lcdClear();
        
        lcdBacklight(32768);
        
        lcdPrintPortrait(" OggBox", 2);
        lcdPrintPortrait("  RevA", 3);
        
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
		for (i = 0; i < 400000; i++)    {	/* Wait a bit. */
			if(gpio_port_read(GPIOD) & 4)
                          gpio_set(RED_LED_PORT, RED_LED_PIN);
                        else
                          gpio_clear(RED_LED_PORT, RED_LED_PIN);
                }
	}

	return 0;
}
