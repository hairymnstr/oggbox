/*
 * This file is part of the oggbox project.
 *
 * Copyright Nathan Dumont 2012-2013 <nathan@nathandumont.com>
 *
 * This firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include "oggbox.h"
#include "block_sd.h"
#include "block.h"

SDCard card = {0, 0, 0, 0};
/**
 *  sd_command - internal function to send a properly formatted command to
 *               to the SD card.
 */
uint16_t sd_command(uint8_t code, uint32_t data, uint8_t chksm) {
  uint16_t c;
  int i;
  
  if(code & 0x80) {
    /* it's an ACMD, so send CMD 55 first */
    spi_xfer(SD_SPI, 0xFF);
  
    spi_xfer(SD_SPI, 0x40 + 55);
    spi_xfer(SD_SPI, 0x00);
    spi_xfer(SD_SPI, 0x00);
    spi_xfer(SD_SPI, 0x00);
    spi_xfer(SD_SPI, 0x00);
    spi_xfer(SD_SPI, 0x01);

    do {
      c = spi_xfer(SD_SPI, 0xFF);
    } while(c == 0xFF);
  }

  spi_xfer(SD_SPI, 0xFF);

  spi_xfer(SD_SPI, 0x40 + (code & 0x7F));
  spi_xfer(SD_SPI, (data >> 24) & 0xFF);
  spi_xfer(SD_SPI, (data >> 16) & 0xFF);
  spi_xfer(SD_SPI, (data >> 8) & 0xFF);
  spi_xfer(SD_SPI, (data & 0xFF));
  spi_xfer(SD_SPI, chksm);

  if(code == CMD12) {
    spi_xfer(SD_SPI, 0xFF);     /* for CMD12 we have to discard a byte */
  }

  for(i=0;i<SD_RETRIES;i++) {
    c = spi_xfer(SD_SPI, 0xFF);
    if(c != 0xFF) {
      return c;
    }
  }

  return c;
}

/**
 * sd_card_reset - performs a software reset on the card to get it ready for
 *                 use.
 **/
uint8_t sd_card_reset() {
  int i;
  uint16_t c;

#ifdef SD_CP
  if(gpio_get(SD_CP_PORT, SD_CP)) {
    card.error = SD_ERR_NOT_PRESENT;
    return -1;
  }
#endif

  /* make sure the card is de-selected */
  gpio_set(SD_PORT, SD_CS);

  /* send more than 80 clock pulses */
  for(i=0;i<20;i++) {
    spi_xfer(SD_SPI, 0xFF);
  }

  /* set chip select low again */
  gpio_clear(SD_PORT, SD_CS);

//  for(i=0;i<1000;i++);

  /* send CMD0 with the correct (pre-computed) CRC */
  c = sd_command(CMD0, 0, 0x95);
  if(c == 0xFF) {
    card.card_type = SD_CARD_ERROR;
    card.error = SD_ERR_NOT_PRESENT;
    return -1;
  }
  //usart_dec_u16(c);
  //usart_puts("\n");

  /* send CMD8 to see if this is a mark 2 SD card */
  c = sd_command(CMD8, 0x000001AA, 0x87);   /* data pattern is used to check voltage levels */
  if(c == 5) {
    card.card_type = SD_CARD_SC;
  } else if(c == 1) {
    /* card is type 2 need to check for voltage/speed corruption */
    if(spi_xfer(SD_SPI, 0xFF) != 0) {
      card.card_type = SD_CARD_ERROR;
      return -1;// SD_CARD_ERROR;
    }
    if(spi_xfer(SD_SPI, 0xFF) != 0) {
      card.card_type = SD_CARD_ERROR;
      return -1;// SD_CARD_ERROR;
    }
    if(spi_xfer(SD_SPI, 0xFF) != 1) {
      card.card_type = SD_CARD_ERROR;
      return -1;// SD_CARD_ERROR;
    }
    if(spi_xfer(SD_SPI, 0xFF) != 0xAA) {
      card.card_type = SD_CARD_ERROR;
      return -1;// SD_CARD_ERROR;
    }
    card.card_type = SD_CARD_HC;                   /* not necessarily but could be, needs further checks */
  } else {
    card.card_type = SD_CARD_ERROR;
    return -1;// SD_CARD_ERROR;                   /* command response was an error. */
  }

  /* send ACMD41 until the card is ready */
  /* TODO: if ACMD41 fails could be MMC in which case CMD1 should be used instead */
  c = 1;
  while(c == 1) {
                                          /* set bit 30 to indicate we are HCSD capable */
    c = sd_command(ACMD41, 1 << 30, 1);   /* checksum is a dummy */
  }

  /* now run a CSD and get some card details (such as HCSD or not etc.) */
  c = sd_command(CMD9, 0, 1);             /* "send CSD" command */
  //usart_hex_u8(c);
  //usart_puts("\n");
  /* got the response to the command, make sure it's 0; no error */
  if(c != 0) {
    card.card_type = SD_CARD_ERROR;
    return -1;
  }

  /* now wait for a start data token (0xFE) */
  do {
    c = spi_xfer(SD_SPI, 0xFF);
  } while(c == 0xFF);
/*  c = sd_read_byte();
  usart_hex_u8(c);
  usart_puts("\n");
  if(c != 0xFF) {
    card->card_type = SD_CARD_ERROR;
    return;
  }
  c = sd_read_byte();
  usart_hex_u8(c);
  usart_puts("\n");
  if(c != 0xFE) {
    card->card_type = SD_CARD_ERROR;
    return;
  }*/

  /* Now deal with the actual content of the CSD */
  c = spi_xfer(SD_SPI, 0xFF);
  /* contains the card type info in top 2 bits */
  /* also determines the structure of the rest of the CSD */
  if(c & 0x40) {
    card.card_type = SD_CARD_HC;
    spi_xfer(SD_SPI, 0xFF);   /* this is always 0x0E no need to check */
    spi_xfer(SD_SPI, 0xFF);   /* this is always 0x00 */
    spi_xfer(SD_SPI, 0xFF);   /* card speed 0x32 or 0x5A, we don't care */
    spi_xfer(SD_SPI, 0xFF);   /* card command class, don't care */
    spi_xfer(SD_SPI, 0xFF);   /* end of class and max block len, don't care */
    spi_xfer(SD_SPI, 0xFF);   /* DSR bit and zeros, don't care */
    c = spi_xfer(SD_SPI, 0xFF);   /* 4 zeros and top 4 bits of size */
    card.size = (c & 0xF) << 16;
    c = spi_xfer(SD_SPI, 0xFF);   /* next byte of size */
    card.size += (c << 8);
    c = spi_xfer(SD_SPI, 0xFF);
    card.size += c;      /* last byte of size */
    card.size <<= 10;    /* want it in 512 blocks but was in 512k */
    spi_xfer(SD_SPI, 0xFF);   /* always 0x7F */
    spi_xfer(SD_SPI, 0xFF);   /* always 0x80 */
    spi_xfer(SD_SPI, 0xFF);   /* always 0x0A */
    spi_xfer(SD_SPI, 0xFF);   /* always 0x40 */
    c = spi_xfer(SD_SPI, 0xFF);   /* write protect flags */
    if(c & 0x30) {
      card.read_only = 1;
    }
    spi_xfer(SD_SPI, 0xFF);   /* checksum */
  } else {
    card.card_type = SD_CARD_SC;
    spi_xfer(SD_SPI, 0xFF);   /* read time, don't care */
    spi_xfer(SD_SPI, 0xFF);   /* more access time */
    spi_xfer(SD_SPI, 0xFF);   /* speed don't care */
    spi_xfer(SD_SPI, 0xFF);   /* ccc part 1 */
    c = spi_xfer(SD_SPI, 0xFF);   /* read block len */
    i = c & 0xF;
    c = spi_xfer(SD_SPI, 0xFF);   /* various flags and top 2 bits of C_SIZE */
    card.size = (c & 0x03) << 10;
    c = spi_xfer(SD_SPI, 0xFF);   /* middle 8 bits of C_SIZE */
    card.size += (c << 2);
    c = spi_xfer(SD_SPI, 0xFF);   /* last two bits and some current info */
    card.size += ((c & 0xC0) >> 6);
    c = spi_xfer(SD_SPI, 0xFF);   /* more current info and top 2 bits of size_mult */
    c = (c << 1) + (spi_xfer(SD_SPI, 0xFF) >> 7);
    c = 1 << ((c & 0x7) + 2);
    card.size++;
    card.size *= c;
    card.size <<= (i - 9);
    spi_xfer(SD_SPI, 0xFF);   /* write protect nonsense */
    spi_xfer(SD_SPI, 0xFF);   /* write info */
    spi_xfer(SD_SPI, 0xFF);   /* more of the same */
    c = spi_xfer(SD_SPI, 0xFF);   /* pre-pressed stuff and WP */
    if(c & 0x30) {
      card.read_only = 1;
    }
    spi_xfer(SD_SPI, 0xFF);   /* the checksum */
  }

  return 0;
}

int block_init() {
  /* need to do the clocks */
  rcc_peripheral_enable_clock(&SD_SPI_APB, SD_RCC_SPI);
  rcc_peripheral_enable_clock(&SD_IO_APB, SD_RCC_IO);

    /* need to do the IO pins */
  gpio_set_mode(SD_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL, SD_CS);
  gpio_set_mode(SD_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, SD_MOSI);
  gpio_set_mode(SD_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, SD_SCK);
  gpio_set_mode(SD_PORT,GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, SD_MISO);
#ifdef SD_WP
  gpio_set_mode(SD_WP_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, SD_WP);
#endif
#ifdef SD_CP
  gpio_set_mode(SD_CP_PORT, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, SD_CP);
#endif

  /* configure the SPI peripheral */
  spi_set_unidirectional_mode(SD_SPI);                          /* we're the only master */
  spi_disable_crc(SD_SPI);                                      /* no CRC for this slave */
  spi_set_dff_8bit(SD_SPI);                                     /* 8-bit dataword-length */
  spi_set_full_duplex_mode(SD_SPI);                             /* not receive-only */
  spi_enable_software_slave_management(SD_SPI);                 /* we want to handle the CS signal 
                                                                   in software */
  spi_set_nss_high(SD_SPI);
  spi_set_baudrate_prescaler(SD_SPI, SPI_CR1_BR_FPCLK_DIV_256); /* PCLOCK/256 as clock */
  spi_set_master_mode(SD_SPI);                                  /* we want to control everything and
                                                                   generate the clock -> master */
  spi_set_clock_polarity_0(SD_SPI);                             /* sck idle state high */
  spi_set_clock_phase_0(SD_SPI);                                /* bit is taken on the second 
                                                                   (rising edge) of sck */
  spi_disable_ss_output(SD_SPI);
  spi_enable(SD_SPI);

  return sd_card_reset();
}

int block_read(blockno_t block, void *buf) {
  int i;
  uint16_t c;
  uint8_t *bp = buf;
  
  if(card.card_type == SD_CARD_SC) {
    block <<= 9;
  }

  c = sd_command(CMD17, block, 1);

  if(c != 0) {
    return c;
  }
  
  do {
    c = spi_xfer(SD_SPI, 0xFF);
  } while(c != 0xFE);

  for(i=0;i<512;i++) {
    *bp++ = spi_xfer(SD_SPI, 0xFF);
  }
  spi_xfer(SD_SPI, 0xFF);
  spi_xfer(SD_SPI, 0xFF);   /* read checksum bytes and dispose of */

  return 0;
}

int block_write(blockno_t block, void *buf) {
  return 0;
}

int block_get_size() {
  return BLOCK_SIZE;
}

int block_get_device_read_only() {
#ifdef SD_WP
  if(gpio_get(SD_WP_PORT, SD_WP))
    return 1;
#endif
  if(card.read_only) {
    return 1;
  } else {
    return 0;
  }
}

int block_sync() {
  return 0;
}

int block_halt() {
  return 0;
}

int block_get_error() {
  return card.error;
}
