/*
 * This file is part of the oggbox project.
 *
 * Copyright Nathan Dumont 2012 <nathan@nathandumont.com>
 *
 * This firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>

#include <stdio.h>

#include "dirent.h"

#include "oggbox.h"
#include "lcd.h"
#include "ui.h"
#include "sdfat.h"
#include "nd_usart.h"
#include "sd.h"
#include "vs1053.h"

extern SDCard card;
extern volatile struct player_status current_track;
extern volatile int current_track_playing;

void gpio_setup(void)
{
  rcc_clock_setup_in_hse_8mhz_out_72mhz();
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);

  /* set the AUX_POWER line to on all the time, turning this off causes an error
     condition on the SPI bus to the CODEC.  In RevB this line is used for sensing
     the battery voltage and should be initialised as off */
  gpio_set(GPIOB, GPIO5);
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
                
  gpio_set_mode(GREEN_LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, RED_LED_PIN | GREEN_LED_PIN);
}

int main(void)
{
  int i;
  char progress[9];
  /* Turn off the JTAG port until next reset */
  AFIO_MAPR = AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_OFF;
  gpio_setup();
  
  /* start up the RTC at 32.768kHz */
  rtc_auto_awake(LSE, 0x7fff);
  
  /* configure the UART for printf debug console */
  usart_clock_setup();
  usart_setup();
        
  lcdInit();
        
  lcdClear();
        
  lcdBacklight(0);
        
  sdfat_init();
  iprintf("Mount SD: %d\r\n", sdfat_mount());
        
  lcdPrintPortrait(" OggBox", 2);
  lcdPrintPortrait("  RevA", 3);
        
  init_codec();
//         demo_codec();
//         play_file_fast("/part01~1.ogg");
//   play_file_fast("/02-THE~1.OGG");
  play_file_fast_async("/02-THE~1.OGG");
  lcdPrintPortrait(" Playing", 5);
  while(current_track_playing) {
    snprintf(progress, 9, "%d", current_track.pos);
    lcdPrintPortrait(progress, 6);
  }
  lcdPrintPortrait("Finished", 5);
  play_file("/magicc~1.ogg");
  
  /* Blink the LED (PC12) on the board. */
  while (1) {
    
    for (i = 0; i < 10000; i++)    {
    }
  }

  return 0;
}
