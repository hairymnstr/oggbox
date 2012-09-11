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

#include "vs1053.h"
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "sdfat.h"
#include "oggbox.h"
//#include <fildes.h>

extern FileS file_num[];
extern MediaFileS media_file;

volatile struct player_status current_track;
volatile int current_track_playing;

/**
 *  PRIVATE FUNCTION
 *  spi_msg - lowest level SPI transfer function.
 *                 for a read call with argument 0xFF
 *                 Makes sure last bit is clocked out
 *                 before returning so CS can be released.
 **/
// uint8_t spi_msg(uint8_t dat) {
//   /* wait for any previous transmission to be done */
//   while((SPI_SR(CODEC_SPI) & SPI_SR_BSY)) {;}
// 
//   /* make sure the RX data flag is clear */
//   if(SPI_SR(CODEC_SPI) & SPI_SR_RXNE) {
//     spi_read(CODEC_SPI);
//   }
// 
//   /* write all 1s out, generates the necessary clocks */
//   spi_write(CODEC_SPI, dat);
// 
//   /* wait until a whole byte has been clocked back in */
//   while((SPI_SR(CODEC_SPI) & SPI_SR_RXNE) == 0) {;}
// 
//   /* return the received byte */
//   return spi_read(CODEC_SPI);
// }

uint8_t spi_msg(uint8_t dat) {
  return spi_xfer(CODEC_SPI, dat);
}

void init_codec() {
  /* enable clock for Aux power and power up the regulators */
  rcc_peripheral_enable_clock(&CODEC_PWR_APB, CODEC_RCC_PWR);
  gpio_set_mode(CODEC_PWR_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, CODEC_PWR);
  gpio_set(CODEC_PWR_PORT, CODEC_PWR);
  
  /* enable SPI1 clock */
  rcc_peripheral_enable_clock(&CODEC_SPI_APB, CODEC_RCC_SPI);
  /* enable clock for the chip select pin */
  rcc_peripheral_enable_clock(&CODEC_IOS_APB, CODEC_RCC_IOS);
  /* enable clock for the RST/DREQ lines */
  rcc_peripheral_enable_clock(&CODEC_IOI_APB, CODEC_RCC_IOI);
  
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);

  /* set the pin modes for the SPI pins */
  gpio_set_mode(CODEC_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, CODEC_CS);
  gpio_set_mode(CODEC_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, CODEC_MOSI);
  gpio_set_mode(CODEC_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, CODEC_SCK);
  gpio_set_mode(CODEC_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, CODEC_MISO);

  /* set the modes for the reset and busy pins */
  gpio_set_mode(CODEC_RST_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, CODEC_RST);
  gpio_set_mode(CODEC_DREQ_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, CODEC_DREQ);

  gpio_clear(CODEC_RST_PORT, CODEC_RST);

  /* configure the SPI port */
  spi_set_unidirectional_mode(CODEC_SPI);
  spi_disable_crc(CODEC_SPI);
  spi_set_dff_8bit(CODEC_SPI);
  spi_set_full_duplex_mode(CODEC_SPI);
  spi_enable_software_slave_management(CODEC_SPI);
  spi_set_nss_high(CODEC_SPI);
  spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_32);
  spi_set_master_mode(CODEC_SPI);
  spi_send_msb_first(CODEC_SPI);
  spi_set_clock_polarity_0(CODEC_SPI);
  spi_set_clock_phase_0(CODEC_SPI);
  spi_disable_ss_output(CODEC_SPI);
  spi_enable(CODEC_SPI);

  /* disable chip select */
  gpio_set(CODEC_PORT, CODEC_CS);

  /* make sure reset is not asserted */
  gpio_set(CODEC_RST_PORT, CODEC_RST);

  return;
}

uint16_t vs1053_SCI_read(uint8_t addr) {
  uint16_t temp;

  gpio_clear(CODEC_PORT, CODEC_CS);

  spi_msg(0x03);

  spi_msg(addr);

  temp = spi_msg(0x00);
  temp <<= 8;

  temp += spi_msg(0x00);

  gpio_set(CODEC_PORT, CODEC_CS);
  return temp;
}

void vs1053_SCI_write(uint8_t addr, uint16_t data) {
  gpio_clear(CODEC_PORT, CODEC_CS);

  spi_msg(0x02);

  spi_msg(addr);

  spi_msg(data >> 8);

  spi_msg(data & 0xFF);

  gpio_set(CODEC_PORT, CODEC_CS);
}

void vs1053_sine_test(uint8_t pitch) {
  gpio_set(CODEC_PORT, CODEC_CS);

  spi_msg(0x53);

  spi_msg(0xEF);

  spi_msg(0x6E);

  spi_msg(pitch);

  spi_msg(0);

  spi_msg(0);

  spi_msg(0);

  spi_msg(0);

  gpio_clear(CODEC_PORT, CODEC_CS);
}

/*void vs1053_get_end_fill_byte() {
  int i;
  vs1053_SCI_write(SCI_WRAMADDR, PARAM_END_FILL_BYTE);
  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}
  codec_params.endFillByte = vs1053_SCI_read(SCI_WRAM);
  return;
}*/

void demo_codec() {
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {;}
  iprintf("initial state 0x%04X\n", vs1053_SCI_read(0));

  vs1053_SCI_write(0x00, 0x0c20);

  vs1053_sine_test(170);
}

void play_file(char *filename) {
  int i, j=0,fn;
  FILE *fr;
//   int s = 1;
  /*

  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {;}
  vs1053_SCI_write(0x00, 0xc00);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {;}
  vs1053_SCI_write(SCI_VOL, 0x3030);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {;}
  iprintf("initial state 0x%04X\n", vs1053_SCI_read(SCI_CLOCKF));*/
//  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}
//  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}


//  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}
//  vs1053_SCI_write(SCI_CLOCKF, 0xA000);
//  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}

  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  vs1053_SCI_write(0x00, 0xc00);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  vs1053_SCI_write(SCI_VOL, 0x3030);
  
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  iprintf("SCI_STATUS = 0x%04X\r\n", vs1053_SCI_read(SCI_STATUS));
  
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  iprintf("initial state 0x%04X\r\n", vs1053_SCI_read(SCI_CLOCKF));
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_256);
  vs1053_SCI_write(SCI_CLOCKF, 0xF800);
  spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_16);
  
  for(i=0;i<1000;i++) {__asm__("nop\n\t");}
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  iprintf("after setup 0x%04X\r\n", vs1053_SCI_read(SCI_CLOCKF));
  
  fr = fopen(filename, "r");
  fn = fileno(fr);
//   iprintf("File No. %d\r\n", fn - FIRST_DISC_FILENO);
  
//   iprintf("==================================\r\n");
//   int k;
//   for(k=0;k<sizeof(FileS);k++) {
//     iprintf("%02X ", ((char *)(&file_num[fn- FIRST_DISC_FILENO]))[k]);
//     if((k+1) % 8 == 0) iprintf("\r\n");
//   }
  
//  l = read(fn, dat, 512);
  while(1) {
    gpio_set(CODEC_PORT, CODEC_CS);
    for(j=0;j<16;j++) {
      while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
      for(i=0;i<32;i+=2) {
        spi_msg(file_num[fn-FIRST_DISC_FILENO].buffer[32*j+i]);
        spi_msg(file_num[fn-FIRST_DISC_FILENO].buffer[32*j+i+1]);
      }
    }
    gpio_clear(CODEC_PORT, CODEC_CS);
//     iprintf("getting sector %d\r\n", s++);
    sdfat_next_sector(fn-FIRST_DISC_FILENO);
  }

}

void play_file_fast(char *filename) {
  int i, j=0, k; //l, fn;
  uint16_t endFillByte;
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  vs1053_SCI_write(0x00, 0xc00);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  vs1053_SCI_write(SCI_VOL, 0x1010);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  iprintf("initial state 0x%04X\n", vs1053_SCI_read(SCI_CLOCKF));
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_256);
  vs1053_SCI_write(SCI_CLOCKF, 0xF800);
  spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_16);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  iprintf("after setup 0x%04X\n", vs1053_SCI_read(SCI_CLOCKF));


//  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}
//  vs1053_SCI_write(SCI_CLOCKF, 0xA000);
//  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}

  sdfat_open_media(filename);

  while(!media_file.near_end) {
    for(k=0;k<4;k++) {
      gpio_set(CODEC_PORT, CODEC_CS);
      for(j=0;j<16;j++) {
        while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
        for(i=0;i<32;i++) {
          spi_msg(media_file.buffer[media_file.active_buffer][512 * k + 32*j + i]);
        }
      }
      gpio_clear(CODEC_PORT, CODEC_CS);
    }
    sdfat_read_media();
  }
  // the current buffer is not full, and is the last
  for(k=0;k<4;k++) {
    gpio_set(CODEC_PORT, CODEC_CS);
    for(j=0;j<16;j++) {
      while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
      for(i=0;i<32;i++) {
        if((512 * k + 32 * j + i) > media_file.file_end) {
          i = 32;
          j = 16;
          k = 4;    /* bust us out of all the nested loops */
        } else {
          spi_msg(media_file.buffer[media_file.active_buffer][512 * k + 32 * j + i]);
        }
      }
    }
    gpio_clear(CODEC_PORT, CODEC_CS);
  }
  /* now need to clean up the fifos and stop the player */
  gpio_clear(CODEC_PORT, CODEC_CS);
  /* fetch the value of endFillByte */
  vs1053_SCI_write(SCI_WRAMADDR, PARAM_END_FILL_BYTE);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  endFillByte = vs1053_SCI_read(SCI_WRAM) & 0xFF;

  gpio_set(CODEC_PORT, CODEC_CS);
  for(i=0;i<65;i++) {
    while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
    for(j=0;j<32;j++) {
      spi_msg(endFillByte);
    }
  }
  gpio_clear(CODEC_PORT, CODEC_CS);
  i = vs1053_SCI_read(SCI_MODE);
  i |= SM_CANCEL;
  vs1053_SCI_write(SCI_MODE, i);
  gpio_set(CODEC_PORT, CODEC_CS);
  j = 0;
  while(j < 2048) {
    while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
    for(i=0;i<32;i++) {
      spi_msg(endFillByte);
    }
    j += 32;
    if(!(vs1053_SCI_read(SCI_MODE) & SM_CANCEL)) {
      break;
    }
  }
  gpio_clear(CODEC_PORT, CODEC_CS);
  if(j >= 2048) {
    /* need to do a software reset */
    vs1053_SCI_write(SCI_MODE, SM_RESET);
  }
  return;
}

void play_file_fast_async(char *filename) {
  //int i, j=0, k; //l, fn;
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  vs1053_SCI_write(0x00, 0xc00);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  vs1053_SCI_write(SCI_VOL, 0x1010);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  iprintf("initial state 0x%04X\n", vs1053_SCI_read(SCI_CLOCKF));
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_256);
  vs1053_SCI_write(SCI_CLOCKF, 0xF800);
  spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_16);
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
  iprintf("after setup 0x%04X\n", vs1053_SCI_read(SCI_CLOCKF));


//  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}
//  vs1053_SCI_write(SCI_CLOCKF, 0xA000);
//  while(!gpio_get(CODEC_CTRL, CODEC_DREQ)) {;}


  current_track.byte_count = 0;
  current_track_playing = 1;
  // now to set up the external interrupt on DREQ
  // may need to trigger the first service in software too as we probably won't get an edge
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);
  
  nvic_enable_irq(NVIC_EXTI3_IRQ);
  nvic_set_priority(NVIC_EXTI3_IRQ, 16);
  
  exti_select_source(EXTI3, CODEC_DREQ_PORT);
  exti_set_trigger(EXTI3, EXTI_TRIGGER_RISING);
  exti_enable_request(EXTI3);
  // generate a software interrupt to get this thing started as there shouldn't be any edges
  sdfat_open_media(filename);
//   iprintf("Generating software interrupt to start playback\r\n");
//   EXTI_SWIER |= EXTI3;
//   exti3_isr();
}

void exti3_isr(void) {
  int i, j;
  uint16_t endFillByte;
  gpio_set(GREEN_LED_PORT, GREEN_LED_PIN);
//   gpio_set(RED_LED_PORT, RED_LED_PIN);
//   iprintf("fill bytes\r\n");
  exti_reset_request(EXTI3);
  iprintf("exti3_isr\r\n");
  if(media_file.buffer_ready[media_file.active_buffer] == 0) {
    iprintf("Buffer %d was ready...\r\n", media_file.active_buffer);
  while(gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {
//     iprintf("loop\r\n");
    gpio_set(CODEC_PORT, CODEC_CS);
    if(!media_file.near_end) {
      for(i=0;i<32;i++) {
        spi_xfer(CODEC_SPI, media_file.buffer[media_file.active_buffer][current_track.byte_count++]);
      }
      if((current_track.byte_count % 512) == 0) {
        gpio_clear(CODEC_PORT, CODEC_CS);
      }
      if(current_track.byte_count == MEDIA_BUFFER_SIZE) {
//         iprintf("reading\r\n");
        sdfat_read_media();
        current_track.byte_count = 0;
        gpio_clear(CODEC_PORT, CODEC_CS);
//         iprintf("swapping...\r\n");
        /* fetch the value of current decode position */
        vs1053_SCI_write(SCI_WRAMADDR, PARAM_POSITION_LO);
        for(i=0;i<150;i++) {__asm__("nop\n\t");}
        current_track.pos = vs1053_SCI_read(SCI_WRAM);
        for(i=0;i<150;i++) {__asm__("nop\n\t");}
        vs1053_SCI_write(SCI_WRAMADDR, PARAM_POSITION_HI);
        for(i=0;i<150;i++) {__asm__("nop\n\t");}
        current_track.pos += vs1053_SCI_read(SCI_WRAM) << 16;
//         iprintf("swap\r\n");
        gpio_set(CODEC_PORT, CODEC_CS);
        if(media_file.buffer_ready[media_file.active_buffer]) {
          iprintf("Breaking out\r\n");
          break;
        }
      }
    } else {
      for(i=0;i<32;i++) {
        if(current_track.byte_count > media_file.file_end) {
          iprintf("ending\r\n");
          // Ought to do this next bit by issuing a player_stop job so that the cleanup code
          // is all in one place and we can power down everything automatically for power saving
          
          /* now need to clean up the fifos and stop the player */
          gpio_clear(CODEC_PORT, CODEC_CS);
          /* fetch the value of endFillByte */
          vs1053_SCI_write(SCI_WRAMADDR, PARAM_END_FILL_BYTE);
          while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
          endFillByte = vs1053_SCI_read(SCI_WRAM) & 0xFF;

          iprintf("End Fill Byte %02X\r\n", endFillByte);
          gpio_set(CODEC_PORT, CODEC_CS);
          for(i=0;i<65;i++) {
            while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
            for(j=0;j<32;j++) {
              spi_xfer(CODEC_SPI, endFillByte);
            }
          }
          gpio_clear(CODEC_PORT, CODEC_CS);
          i = vs1053_SCI_read(SCI_MODE);
          i |= SM_CANCEL;
          vs1053_SCI_write(SCI_MODE, i);
          gpio_set(CODEC_PORT, CODEC_CS);
          j = 0;
          while(j < 2048) {
            while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
            for(i=0;i<32;i++) {
              spi_xfer(CODEC_SPI, endFillByte);
            }
            j += 32;
            if(!(vs1053_SCI_read(SCI_MODE) & SM_CANCEL)) {
              break;
            }
          }
          gpio_clear(CODEC_PORT, CODEC_CS);
          if(j >= 2048) {
            /* need to do a software reset */
            vs1053_SCI_write(SCI_MODE, SM_RESET);
          }
          iprintf("End Fill Byte %02X\r\n", endFillByte);
          
          current_track_playing = 0;
          
          exti_reset_request(EXTI3);
          exti_disable_request(EXTI3);
          nvic_disable_irq(NVIC_EXTI3_IRQ);
          return;
        } else {
          spi_xfer(CODEC_SPI, media_file.buffer[media_file.active_buffer][current_track.byte_count++]);
        }
      }
    }
  }
  }

  gpio_clear(GREEN_LED_PORT, GREEN_LED_PIN);
}
