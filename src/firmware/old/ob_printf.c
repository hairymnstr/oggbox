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
#include <stdarg.h>
#include "ob_string.h"
#include "config.h"

int ob_printf(const char *format, ...) {
  int i=0, j;
  va_list args;
  int pad;
  char pad_char;
  const char *fc = format;
  char buf[33], *bc;
  
  va_start(args, format);

  while((*fc) != 0) {
    if((*fc) == '%') {
      fc++;
      pad = 0;
      pad_char = ' ';
      if(*fc == '0') {
        pad_char = '0';
        fc++;
      }
      while(((*fc) >= '0') && ((*fc) <= '9')) {
        pad *= 10;
        pad += ((*fc++) - '0');
      }
      switch(*fc) {
        case '%':
          usart_send(DBG_URT, *fc++);
          break;
        case 'd':
          ob_itoa(va_arg(args, int), buf, 10);
          bc = buf;
          goto output_buffer;
        case 'x':
          ob_itoa(va_arg(args, int), buf, 16);
          bc = buf;
          goto output_buffer;
        case 'X':
          ob_itoa(va_arg(args, int), buf, 16);
          ob_strtuc(buf);

output_buffer:
          bc = buf;
          for(j=0;j<pad - ob_strlen(buf);j++) {
            usart_send(DBG_URT, pad_char);
            i++;
          }
          while((*bc) != 0) {
            usart_send(DBG_URT, *bc++);
            i++;
          }
          fc++;
          break;
        case 'p':
          ob_itoa(va_arg(args, int), buf, 16);
          bc = buf;
          usart_send(DBG_URT, '0');
          i++;
          usart_send(DBG_URT, 'x');
          i++;
          while((*bc) != 0) {
            usart_send(DBG_URT, *bc++);
            i++;
          }
          fc++;
          break;
        default:
          usart_send(DBG_URT, *fc++);
          i++;
      }
    } else {
      usart_send(DBG_URT, *fc++);
      i++;
    }
    va_end(args);
  }
  return i;
}

