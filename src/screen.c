/*
 * This file is part of the oggbox project.
 *
 * Copyright (C) 2012 Nathan Dumont <nathan@nathandumont.com>
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

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/f1/bkp.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>

#include <stdint.h>
#include <stdio.h>

#include "screen.h"
#include "config.h"

extern const char *font[];
extern const char *font_portrait[];

unsigned char frame_buffer[128*64/8];
int frame_cursor = 0;

void lcdCommand(unsigned char cmd) {
//   int i;
  //while(lcdStatus() & LCD_BUSY) {__asm__("nop\n\t");}
  //gpio_set_mode(LCD_DATA_PORT, GPIO_MODE_OUTPUT_10_MHZ,
  //              GPIO_CNF_OUTPUT_PUSHPULL, LCD_DATA_MASK);
  //gpio_clear(LCD_DATA_PORT, LCD_DATA_MASK);
  //gpio_set(LCD_DATA_PORT, cmd);

  // write mode = low
  //gpio_clear(LCD_RW_PORT, LCD_RW_PIN);
  // command mode = low
  gpio_clear(LCD_DC_PORT, LCD_DC_PIN);

  gpio_clear(LCD_CS_PORT, LCD_CS_PIN);
  //gpio_set(LCD_E_PORT, LCD_E_PIN);

  spi_write(LCD_SPI, cmd);
  
//   for(i=0;i<100;i++) {__asm__("nop\n\t");}
  while((SPI_SR(LCD_SPI) & SPI_SR_BSY)) {__asm__("nop");}
  //gpio_clear(LCD_E_PORT, LCD_E_PIN);
  gpio_set(LCD_CS_PORT, LCD_CS_PIN);

  //gpio_set_mode(LCD_DATA_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, LCD_DATA_MASK);
}

void lcdData(unsigned char d) {
//   int i;
  //while(lcdStatus() & LCD_BUSY) {__asm__("nop\n\t");}
  //gpio_set_mode(LCD_DATA_PORT, GPIO_MODE_OUTPUT_10_MHZ,
  //              GPIO_CNF_OUTPUT_PUSHPULL, LCD_DATA_MASK);
  //gpio_clear(LCD_DATA_PORT, LCD_DATA_MASK);
  //gpio_set(LCD_DATA_PORT, d);

  // write mode = low
  //gpio_clear(LCD_RW_PORT, LCD_RW_PIN);
  // data mode = high
  gpio_set(LCD_DC_PORT, LCD_DC_PIN);

  gpio_clear(LCD_CS_PORT, LCD_CS_PIN);
  //gpio_set(LCD_E_PORT, LCD_E_PIN);
  spi_write(LCD_SPI, d);
  
//   for(i=0;i<100;i++) {__asm__("nop\n\t");}
  while((SPI_SR(LCD_SPI) & SPI_SR_BSY)) {__asm__("nop");}
  //gpio_clear(LCD_E_PORT, LCD_E_PIN);
  gpio_set(LCD_CS_PORT, LCD_CS_PIN);

  //gpio_set_mode(LCD_DATA_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, LCD_DATA_MASK);
}

// unsigned char lcdStatus() {
//   unsigned char rv;
//   int i;
//   gpio_set(LCD_RW_PORT, LCD_RW_PIN);
//   gpio_clear(LCD_DC_PORT, LCD_DC_PIN);
//   
//   gpio_clear(LCD_CS_PORT, LCD_CS_PIN);
//   gpio_set(LCD_E_PORT, LCD_E_PIN);
//   
//   for(i=0;i<100;i++) {__asm__("nop\n\t");}
//   
//   rv = gpio_port_read(LCD_DATA_PORT) & LCD_DATA_MASK;
//   gpio_clear(LCD_E_PORT, LCD_E_PIN);
//   gpio_set(LCD_CS_PORT, LCD_CS_PIN);
//   return rv;
// }

void screen_init() {
  int i;
  // hardware layout specific optimisations here
  // if you change the pinout these need to be changed
  rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI3EN);
  
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, 
                LCD_RST_PIN | LCD_CS_PIN | LCD_DC_PIN);

  gpio_set_mode(LCD_BL_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, LCD_BL_PIN);
  
  gpio_set_mode(LCD_MOSI_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, LCD_MOSI_PIN);
  gpio_set_mode(LCD_SCK_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, LCD_SCK_PIN);
  
  gpio_clear(LCD_RST_PORT, LCD_RST_PIN);
  gpio_set(LCD_CS_PORT, LCD_CS_PIN);
  gpio_set(LCD_DC_PORT, LCD_DC_PIN);
  gpio_clear(LCD_BL_PORT, LCD_BL_PIN);
  
  /* Set up the serial port */
    /* configure the SPI peripheral */
  spi_set_unidirectional_mode(LCD_SPI); /* we want to send only */
  spi_disable_crc(LCD_SPI); /* no CRC for this slave */
  spi_set_dff_8bit(LCD_SPI); /* 8-bit dataword-length */
  spi_set_full_duplex_mode(LCD_SPI);    /* otherwise it's read only */
  spi_enable_software_slave_management(LCD_SPI); /* we want to handle the CS signal in software */
  spi_set_nss_high(LCD_SPI);
  spi_set_baudrate_prescaler(LCD_SPI, SPI_CR1_BR_FPCLK_DIV_4); /* PCLOCK/256 as clock */
  spi_set_master_mode(LCD_SPI); /* we want to control everything and generate the clock -> master */
  spi_set_clock_polarity_1(LCD_SPI); /* sck idle state high */
  spi_set_clock_phase_1(LCD_SPI); /* bit is taken on the second (rising edge) of sck */
  spi_disable_ss_output(LCD_SPI);
  spi_enable(LCD_SPI);
    
  for(i=0;i<10000;i++) {__asm__("nop\n\t");}
  
  gpio_set(LCD_RST_PORT, LCD_RST_PIN);
  
  for(i=0;i<10000;i++) {__asm__("nop\n\t");}
  
  lcdCommand(LCD_BIAS_SEVENTH);
  lcdCommand(LCD_DIRECTION_FWD);
  lcdCommand(LCD_COMMON_FWD);
  lcdCommand(LCD_VREG_SET);
  lcdCommand(LCD_CONTRAST_HI);
  lcdCommand(LCD_CONTRAST_LO(lcd_get_contrast()));
  lcdCommand(LCD_POWER_SETUP);
  lcdCommand(LCD_DISPLAY_ON);
  
}

int lcd_shutdown() {
  gpio_clear(LCD_RST_PORT, LCD_RST_PIN);
  gpio_clear(LCD_BL_PORT, LCD_BL_PIN);
  spi_disable(LCD_SPI);
  rcc_peripheral_disable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);
  return 0;
}

void lcdBacklight(unsigned short on) {
  if(on) {
    gpio_set(LCD_BL_PORT, LCD_BL_PIN);
  } else {
    gpio_clear(LCD_BL_PORT, LCD_BL_PIN);
  }
}

void lcd_set_contrast(unsigned char contrast) {
  contrast = contrast & 0x3F;
  BKP_DR1 = contrast;
  lcdCommand(LCD_CONTRAST_HI);
  lcdCommand(LCD_CONTRAST_LO(contrast));
}

unsigned char lcd_get_contrast() {
  unsigned char con;
  
  con = BKP_DR1;
  if(con == 0) {
    con = 32;
  }
  return con;
}

void lcdClear() {
  int i, j;
  
  for(i=0;i<8;i++) {
    lcdCommand(LCD_PAGE_SET(i));
    lcdCommand(LCD_COLUMN_SET_HI(0));
    lcdCommand(LCD_COLUMN_SET_LO(0));
    for(j=0;j<128;j++) {
      lcdData(0x00);
    }
  }
  lcdCommand(LCD_PAGE_SET(0));
  lcdCommand(LCD_COLUMN_SET_HI(0));
  lcdCommand(LCD_COLUMN_SET_LO(0));
}

void lcdPrint(char *msg, char line) {
  int i, j;
  for(i=0;i<21;i++) {
    if(msg[i] == 0) {
      break;
    }
    lcdCommand(LCD_PAGE_SET(7-line));
    lcdCommand(LCD_COLUMN_SET_HI(i*6));
    lcdCommand(LCD_COLUMN_SET_LO(i*6));
    for(j=0;j<6;j++) {
      lcdData(font[msg[i]][j]);
    }
  }
}

// void lcdBlit(uint8_t *img, unsigned char rows, unsigned char cols, unsigned char x, unsigned char y) {
//   lcdBlitPortrait(img, rows, cols, x, y);
// }

// void lcdBlitPortrait(uint8_t *img, unsigned char rows, unsigned char cols,
//                      unsigned char x, unsigned char y) {
//   int j;
//   lcdCommand(LCD_PAGE_SET(x/8));
//   lcdCommand(LCD_COLUMN_SET_HI(4 + y));
//   lcdCommand(LCD_COLUMN_SET_LO(4 + y));
//   for(j=0;j<8;j++) {
//     lcdData(img[j]);
//   }
// }

void frameCharAt(uint8_t x, uint8_t y, char c) {
  int i;
  y &= 7;
  for(i=0;i<FONT_WIDTH;i++) {
    if(x + i < SCREEN_WIDTH) {
      frame_buffer[y*SCREEN_WIDTH+x+i] = font[(unsigned char)c][i];
    }
  }
}

void frame_char_at_portrait(uint8_t x, uint8_t y, char c) {
  uint8_t oldval;
  int i;
  int shift;
  // x = width (64 pixels)
  // y = height (128 pixels)
  shift = x % 8;
  for(i=0;i<8;i++) {
    if(shift < 2) {
      // all bits are in the same byte
      oldval = frame_buffer[(7 - x/8) * 128 + y + i];
      oldval &= ~(0x3f << shift);
      oldval |= (font_portrait[(unsigned char)c][i] & 0x3f) << shift;
      frame_buffer[(7 - x/8) * 128 + y + i] = oldval;
    } else {
      // character spans two pages
      oldval = frame_buffer[(7 - x/8) * 128 + y + i];
      oldval &= ~(0x3f << shift);
      oldval |= (font_portrait[(unsigned char)c][i] & 0x3f) << shift;
      frame_buffer[(7 - x/8) * 128 + y + i] = oldval;
      
      oldval = frame_buffer[(7 - x/8 - 1) * 128 + y + i];
      oldval &= ~(0x3f >> (8 - shift));
      oldval |= (font_portrait[(unsigned char)c][i] & 0x3f) >> (8 - shift);
      frame_buffer[(7 - x/8 - 1) * 128 + y + i] = oldval;
    }
  }
}

void frame_clear() {
  int i;
  frame_cursor = 0;
  for(i=0;i<1024;i++) {
    frame_buffer[i] = 0;
  }
}

// void framePrint(const char *msg) {
// // 
// }

void frame_print_at(uint8_t x, uint8_t y, const char *msg) {
  while(*msg) {
    frame_char_at_portrait(x,y,*msg++);
    x += 6;
  }
}

// void frame_print_at(uint8_t x, uint8_t y, const char *msg) {
//   y = y / 8;
//   
//   while(*msg) {
//     frameCharAt(x,y,*msg);
//     x += FONT_WIDTH;
//     msg++;
//   }
// }

void frame_bar_display(uint8_t x, uint8_t y, uint8_t len, uint8_t percent) {
  int i, filled;

  y = y >> 3;
  if((len + x) > SCREEN_WIDTH) {
    len = SCREEN_WIDTH - x;
  }

  filled = len;
  filled *= percent;
  filled /= 100;
  
  for(i=0;i<filled;i++) {
//     if(i < filled) {
      frame_buffer[y*SCREEN_WIDTH+x+i] = 0x7e;
//     } else {
//       frame_buffer[y*SCREEN_WIDTH+x+i] = 0x00;
//     }
  }
}

void frame_vline_at(uint8_t x, uint8_t y, uint8_t len) {
  int row;
  
  row = y/8;
  
  frame_buffer[row*SCREEN_WIDTH+x] = (1 << (y %8)) - 1;
  len -= (y % 8);
  while(len > 8) {
    row++;
    frame_buffer[row*SCREEN_WIDTH+x] = 0xff;
    len -= 8;
  }
  if(len > 0) {
    row++;
    frame_buffer[row*SCREEN_WIDTH+x] = 0xff ^ ((1 << (y %8)) - 1);
  }
}

void frame_show() {
  int i, j;
  for(i=0;i<8;i++) {
    lcdCommand(LCD_PAGE_SET(i));
    lcdCommand(LCD_COLUMN_SET_HI(0));
    lcdCommand(LCD_COLUMN_SET_LO(4));
    for(j=0;j<128;j++) {
      lcdData(frame_buffer[(7-i)*SCREEN_WIDTH+j]);
    }
  }
  lcdCommand(LCD_PAGE_SET(0));
  lcdCommand(LCD_COLUMN_SET_HI(0));
  lcdCommand(LCD_COLUMN_SET_LO(4));
}

void lcd_splash(const char *image[]) {
  int i, j;
  for(i=0;i<8;i++) {
    lcdCommand(LCD_PAGE_SET(i));
    lcdCommand(LCD_COLUMN_SET_HI(0));
    lcdCommand(LCD_COLUMN_SET_LO(4));
    for(j=0;j<128;j++) {
      lcdData(image[(7-i)][j]);
    }
  }
  lcdCommand(LCD_PAGE_SET(0));
  lcdCommand(LCD_COLUMN_SET_HI(0));
  lcdCommand(LCD_COLUMN_SET_LO(4));
}
