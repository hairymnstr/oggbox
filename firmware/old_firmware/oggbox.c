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
#include <libopencm3/stm32/f1/rtc.h>
#include <stdio.h>

#include <unistd.h>

#include "dirent.h"

#include "oggbox.h"
#include "lcd.h"
#include "ui.h"
#include "block.h"
#include "nd_usart.h"
#include "vs1053.h"
#include "ogg_meta.h"
#include "fat.h"
#include "partition.h"

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
//   gpio_set(GPIOB, GPIO5);
//   gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
//                 GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
  gpio_set_mode(AUX_POWER_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, AUX_POWER_PIN);
                
  gpio_set_mode(GREEN_LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, RED_LED_PIN | GREEN_LED_PIN);
}

void aux_power_on() {
  gpio_set(AUX_POWER_PORT, AUX_POWER_PIN);
}

int main(void)
{
  int i;
  int r;
  int len;
  char progress[9];
  uint8_t buffer[512];
  struct partition *part_list;
  int mounted = 0;
  
  /* Turn off the JTAG port until next reset */
  AFIO_MAPR = AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_OFF;
  gpio_setup();
  
  /* start up the RTC at 32.768kHz */
  rtc_auto_awake(LSE, 0x7fff);
  
  /* configure the UART for printf debug console */
  usart_clock_setup();
  usart_setup();
        
  aux_power_on();
  //lcdInit();
        
  //lcdClear();
        
  //lcdBacklight(0);
        
  iprintf("==================================================\r\n");
  r = block_init();
  iprintf("Block init: %d\r\n", r);
  if(r == 0) {
    r = fat_mount(0, block_get_volume_size(), 0);
    iprintf("Mount card root: %d\r\n", r);
    if(r != 0) {
      block_read(0, buffer);
      r = read_partition_table(buffer, block_get_volume_size(), &part_list);
      iprintf("Found %d partitions\r\n", r);
      if(r > 0) {
        for(i=0;i<r;i++) {
          if(fat_mount(part_list[i].start, part_list[i].length, part_list[i].type) == 0) {
            iprintf("Mounted partition %d\r\n", i);
            mounted = 1;
            break;
          }
        }
      }
    } else {
      mounted = 1;
    }
  }
  
  iprintf("Mount: %d\r\n", mounted);
  //iprintf("Mount SD: %d\r\n", sdfat_mount());
        
  //lcdPrintPortrait(" OggBox", 2);
  //lcdPrintPortrait("  RevA", 3);
  
  init_codec();
//   demo_codec();
//         play_file_fast("/part01~1.ogg");
//   play_file_fast("/02-THE~1.OGG");
  len = ogg_track_length_millis("/outro.OGG");
  snprintf(progress, 9, "%d", len);
  iprintf("%s\r\n", progress);
//   lcdPrintPortrait(progress, 8);
  play_file_fast_async("/outro.OGG");
//   lcdPrintPortrait(" Playing", 5);
  while(current_track_playing) {
    if(((current_track.pos * 100) / len > 0) && ((current_track.pos * 100) / len < 100)) {
      snprintf(progress, 9, "%ld%%", (current_track.pos * 100) / len );
      iprintf("%s\r\n", progress);
//       lcdPrintPortrait(progress, 6);
    }
  }
//   lcdPrintPortrait("Finished", 5);
//   iprintf("Finished\r\n");
  play_file_fast_async("/magicc~1.ogg");
  
  /* Blink the LED (PC12) on the board. */
  while (1) {
    gpio_toggle(RED_LED_PORT, RED_LED_PIN);
    for (i = 0; i < 10000000; i++) {
      __asm__("nop\n");
    }
  }

  return 0;
}
