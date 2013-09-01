/*
 * This file is part of the oggbox project.
 *
 * Copyright Nathan Dumont 2013 <nathan@nathandumont.com>
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

#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ogg_meta.h"
#include "vs1053.h"
#include "config.h"


#define PLAYER_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define PLAYER_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 2000)

volatile struct player_status current_track;
volatile int current_track_playing;
FILE *media_fd;
ssize_t media_byte_len;
extern xQueueHandle player_queue;

const char *jimmy_album_title = "Bubbleino EP";
const char *jimmy_artist_name = "20lb Sounds";
const char *jimmy_track_title = "Jimmy Carter";

/**
 *  PRIVATE FUNCTION
 *  spi_msg - lowest level SPI transfer function.
 *                 for a read call with argument 0xFF
 *                 Makes sure last bit is clocked out
 *                 before returning so CS can be released.
 **/
uint8_t spi_msg(uint8_t dat) {
  return spi_xfer(CODEC_SPI, dat);
}

void init_codec() {
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

void demo_codec() {
  while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {;}
  iprintf("initial state 0x%04X\r\n", vs1053_SCI_read(0));

  vs1053_SCI_write(0x00, 0x0c20);

  vs1053_sine_test(170);
}

static void player_task(void *parameters __attribute__((unused))) {
  int i, j;
  uint16_t endFillByte;
  struct player_job job_to_do;
  struct meta metainfo;
  int jobs_serviced;
//   char filename[] = "/20lbSo~1.ogg";
  char filename[] = "/06-Fol~1.ogg";
  
  // do setup stuff
  init_codec();
  
  while(1) {
    while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
    vs1053_SCI_write(0x00, 0xc00);
    while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
    vs1053_SCI_write(SCI_VOL, 0x1010);
    while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
    iprintf("initial state 0x%04X\r\n", vs1053_SCI_read(SCI_CLOCKF));
    while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
    spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_256);
    vs1053_SCI_write(SCI_CLOCKF, 0xF800);
    spi_set_baudrate_prescaler(CODEC_SPI, SPI_CR1_BR_FPCLK_DIV_32);
    while(!gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {__asm__("nop\n\t");}
    iprintf("after setup 0x%04X\r\n", vs1053_SCI_read(SCI_CLOCKF));

    current_track_playing = 1;

    media_fd = fopen(filename, "rb");
    if(media_fd == 0) {
      iprintf("Failed to open media file: %s\r\n", strerror(errno));
      while(1) {
        vTaskDelay(1000);
      }
    }
    
    current_track.length = ogg_track_length_millis(media_fd);
    fseek(media_fd, 0, SEEK_SET);
    if(read_standard_tags(media_fd, &metainfo) == 0) {
      current_track.artist_name = metainfo.artist;
      current_track.album_title = metainfo.album;
      current_track.track_title = metainfo.title;
    } else {
      strncpy(metainfo.artist, "Unknown", META_STR_LEN);
      strncpy(metainfo.album, "Unknown", META_STR_LEN);
      strncpy(metainfo.title, filename, META_STR_LEN);
      current_track.artist_name = metainfo.artist;
      current_track.album_title = metainfo.album;
      current_track.track_title = metainfo.title;
    }
    
    fseek(media_fd, 0, SEEK_END);
    media_byte_len = ftell(media_fd);
    fseek(media_fd, 0, SEEK_SET);
    
    while(current_track_playing) {
      gpio_set(GREEN_LED_PORT, GREEN_LED_PIN);
      while(gpio_get(CODEC_DREQ_PORT, CODEC_DREQ)) {
        gpio_set(CODEC_PORT, CODEC_CS);
        if(ftell(media_fd) < (media_byte_len - 32)) {
          for(i=0;i<32;i++) {
            spi_xfer(CODEC_SPI, fgetc(media_fd));
          }
          if((ftell(media_fd) % 512) == 0) {
            gpio_clear(CODEC_PORT, CODEC_CS);
          }
          if((ftell(media_fd) % 4096) == 0) {
            gpio_clear(CODEC_PORT, CODEC_CS);
            
            // take this opportunity to do any jobs that involve the control registers in the
            // codec chip
            /* fetch the value of current decode position */
            vs1053_SCI_write(SCI_WRAMADDR, PARAM_POSITION_LO);
            for(i=0;i<150;i++) {__asm__("nop\n\t");}
            current_track.position = vs1053_SCI_read(SCI_WRAM);
            for(i=0;i<150;i++) {__asm__("nop\n\t");}
            vs1053_SCI_write(SCI_WRAMADDR, PARAM_POSITION_HI);
            for(i=0;i<150;i++) {__asm__("nop\n\t");}
            current_track.position += vs1053_SCI_read(SCI_WRAM) << 16;
            for(i=0;i<150;i++) {__asm__("nop\n\t");}
            gpio_set(CODEC_PORT, CODEC_CS);
            
            jobs_serviced = 0;
            // now check the job list for any other jobs that need doing
            while(xQueueReceive(player_queue, &job_to_do, 0) == pdTRUE) {
              jobs_serviced++;
              
              switch(job_to_do.type) {
                case PLAYER_VOLUME_COMMAND:
                  // set the volume of the codec chip
                  vs1053_SCI_write(SCI_VOL, job_to_do.data);
                  for(i=0;i<150;i++) {__asm__("nop\n\t");}
                  
                  break;
                default:
                  // dunno what you mean :P
                  break;
              }
              
              if(jobs_serviced == 5) {
                break;          // don't do too many jobs in one go, don't want the player to stutter
              }
            }
          }
        } else {
          for(i=0;i<32;i++) {
            if(feof(media_fd)) {
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
              i=50;
            } else {
              spi_xfer(CODEC_SPI, fgetc(media_fd));
            }
          }
          if(!current_track_playing) {
            break;
          }
        }
      }

      gpio_clear(GREEN_LED_PORT, GREEN_LED_PIN);
      if(current_track_playing) {
        vTaskDelay(10);                // yield for a bit
      }
    }
    fclose(media_fd);
  }
  // shouldn't ever exit that loop
}


void start_player_task() {
  
  xTaskCreate( player_task, (const signed char * const)"PLAYER", PLAYER_TASK_STACK_SIZE, NULL, PLAYER_TASK_PRIORITY, NULL);
  
}

char *player_get_artist() {
  return current_track.artist_name;
}

char *player_get_title() {
  return current_track.track_title;
}

char *player_get_album() {
  return current_track.album_title;
}

uint32_t player_get_length() {
  return current_track.length;
}

uint32_t player_get_position() {
  return current_track.position;
}

int player_is_playing() {
  if(current_track_playing) {
    return 1;
  }
  return 0;
}
