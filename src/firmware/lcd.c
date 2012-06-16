/*
 * This file is part of the oggbox project.
 *
 * Copyright (C) 2012 Nathan Dumont <nathan@nathandumont.com>
 *
 * Oggbox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Oggbox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Oggbox.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "oggbox.h"
#include "lcd.h"

extern const char *font[];

void lcdCommand(unsigned char cmd) {
  int i;
  while(lcdStatus() & LCD_BUSY) {__asm__("nop\n\t");}
  gpio_set_mode(LCD_DATA_PORT, GPIO_MODE_OUTPUT_10_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, LCD_DATA_MASK);
  gpio_clear(LCD_DATA_PORT, LCD_DATA_MASK);
  gpio_set(LCD_DATA_PORT, cmd);

  // write mode = low
  gpio_clear(LCD_RW_PORT, LCD_RW_PIN);
  // command mode = low
  gpio_clear(LCD_DC_PORT, LCD_DC_PIN);

  gpio_clear(LCD_CS_PORT, LCD_CS_PIN);
  gpio_set(LCD_E_PORT, LCD_E_PIN);

  for(i=0;i<100;i++) {__asm__("nop\n\t");}

  gpio_clear(LCD_E_PORT, LCD_E_PIN);
  gpio_set(LCD_CS_PORT, LCD_CS_PIN);

  gpio_set_mode(LCD_DATA_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, LCD_DATA_MASK);
}

void lcdData(unsigned char d) {
  int i;
  while(lcdStatus() & LCD_BUSY) {__asm__("nop\n\t");}
  gpio_set_mode(LCD_DATA_PORT, GPIO_MODE_OUTPUT_10_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, LCD_DATA_MASK);
  gpio_clear(LCD_DATA_PORT, LCD_DATA_MASK);
  gpio_set(LCD_DATA_PORT, d);

  // write mode = low
  gpio_clear(LCD_RW_PORT, LCD_RW_PIN);
  // data mode = high
  gpio_set(LCD_DC_PORT, LCD_DC_PIN);

  gpio_clear(LCD_CS_PORT, LCD_CS_PIN);
  gpio_set(LCD_E_PORT, LCD_E_PIN);

  for(i=0;i<100;i++) {__asm__("nop\n\t");}

  gpio_clear(LCD_E_PORT, LCD_E_PIN);
  gpio_set(LCD_CS_PORT, LCD_CS_PIN);

  gpio_set_mode(LCD_DATA_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, LCD_DATA_MASK);
}

volatile unsigned char lcdStatus() {
  unsigned char rv;
  int i;
  gpio_set(LCD_RW_PORT, LCD_RW_PIN);
  gpio_clear(LCD_DC_PORT, LCD_DC_PIN);
  
  gpio_clear(LCD_CS_PORT, LCD_CS_PIN);
  gpio_set(LCD_E_PORT, LCD_E_PIN);
  
  for(i=0;i<100;i++) {__asm__("nop\n\t");}
  
  rv = gpio_port_read(LCD_DATA_PORT) & LCD_DATA_MASK;
  gpio_clear(LCD_E_PORT, LCD_E_PIN);
  gpio_set(LCD_CS_PORT, LCD_CS_PIN);
  return rv;
}

void lcdInit() {
  int i;
  // hardware layout specific optimisations here
  // if you change the pinout these need to be changed
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
  
  // timer 2 for backlight dimming
  rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_TIM2EN);
  
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_10_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, 
                LCD_RST_PIN | LCD_E_PIN | LCD_RW_PIN | LCD_CS_PIN);
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL,
                LCD_DC_PIN);
  gpio_set_mode(LCD_BL_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, LCD_BL_PIN);
  
  gpio_clear(LCD_RST_PORT, LCD_RST_PIN);
  gpio_set(LCD_CS_PORT, LCD_CS_PIN);
  gpio_set(LCD_DC_PORT, LCD_DC_PIN);
  gpio_clear(LCD_RW_PORT, LCD_RW_PIN);
  gpio_clear(LCD_E_PORT, LCD_E_PIN);
  
  /* === Setup PWM peripherals for brightness === */
   /* global settings for timer 1 */
  TIM2_CR1 = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
  TIM2_ARR = 65535;
  TIM2_PSC = 0;
  TIM2_EGR = TIM_EGR_UG;

   /* timer 1 channel 2 (screen brightness) */
  TIM2_CCMR2 |= TIM_CCMR2_OC3M_PWM1 | TIM_CCMR2_OC3PE;
  TIM2_CCER |= TIM_CCER_CC3E;
  TIM2_CCR3 = 0;

  TIM2_CR1 |= TIM_CR1_ARPE;
  TIM2_CR1 |= TIM_CR1_CEN;

//   TIM2_BDTR |= TIM_BDTR_MOE;
  
  for(i=0;i<10000;i++) {__asm__("nop\n\t");}
  
  gpio_set(LCD_RST_PORT, LCD_RST_PIN);
  
  for(i=0;i<10000;i++) {__asm__("nop\n\t");}
  
  lcdCommand(LCD_BIAS_SEVENTH);
  lcdCommand(LCD_DIRECTION_FWD);
  lcdCommand(LCD_COMMON_FWD);
  lcdCommand(LCD_VREG_SET);
  lcdCommand(LCD_CONTRAST_HI);
  lcdCommand(LCD_CONTRAST_LO(32));
  lcdCommand(LCD_POWER_SETUP);
  lcdCommand(LCD_DISPLAY_ON);
  
}

void lcdBacklight(unsigned short level) {
  TIM2_CCR3 = level;
}

void lcdContrast(unsigned char contrast) {
  contrast = contrast & 0x3F;
  lcdCommand(LCD_CONTRAST_HI);
  lcdCommand(LCD_CONTRAST_LO(contrast));
}

void lcdClear() {
  int i, j;
  
  for(i=0;i<8;i++) {
    lcdCommand(LCD_PAGE_SET(i));
    lcdCommand(LCD_COLUMN_SET_HI(4));
    lcdCommand(LCD_COLUMN_SET_LO(4));
    for(j=0;j<128;j++) {
      lcdData(0x00);
    }
  }
  lcdCommand(LCD_PAGE_SET(0));
  lcdCommand(LCD_COLUMN_SET_HI(4));
  lcdCommand(LCD_COLUMN_SET_LO(4));
}

void lcdPrintPortrait(char *msg, char line) {
  int i, j;
  for(i=0;i<8;i++) {
    if(msg[i] == 0) {
      break;
    }
    lcdCommand(LCD_PAGE_SET(i));
    lcdCommand(LCD_COLUMN_SET_HI(4 + line * 8));
    lcdCommand(LCD_COLUMN_SET_LO(4 + line * 8));
    for(j=0;j<8;j++) {
      lcdData(font[msg[i]][j]);
    }
  }
}
  
