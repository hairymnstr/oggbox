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

#include "config.h"

void usart_putc(char c) {
  usart_send(USART2, c);
}

u8 usart_getc() {
  return usart_recv(USART2);
}

void usart_puts(char *s) {
  while(*s != 0) {
    usart_putc(*s++);
  }
}

int ob_printf(const char *format, ...) {
  int i=0;
  while(*format != 0) {
    usart_send(DBG_URT, *format++);
    i++;
  }
  return i;
}

