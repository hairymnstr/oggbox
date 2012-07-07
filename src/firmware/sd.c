#include "sd.h"
// #include "nd_usart.h"
// #include "libopenstm32/gpio.h"
#include <libopencm3/stm32/f1/gpio.h>
// #include "libopenstm32/spi.h"
#include <libopencm3/stm32/spi.h>
// #include "libopenstm32/rcc.h"
#include <libopencm3/stm32/f1/rcc.h>
// #include "libopenstm32/dma.h"
#include <libopencm3/stm32/f1/dma.h>
// #include "libopenstm32/nvic.h"
#include <libopencm3/stm32/nvic.h>
#include <libopencm3/stm32/f1/nvic_f1.h>

#include <stdint.h>

#include "mbr.h"
#include "config.h"
// #include <stdio.h>

extern SDCard card;

char *dma_buffer_addr;
int dma_block_no;
int dma_total_blocks;
unsigned int *dma_report_done;
char *garbage = "\xFF";
int dma_state;

/**
 *  sd_transfer_byte - internal low level transfer byte function
 */
uint8_t sd_transfer_byte(u8 data) {
  spi_write(SD_SPI, data);

  while((SPI_SR(SD_SPI) & SPI_SR_RXNE) == 0) {;}

  return spi_read(SD_SPI);
}

/**
 *  sd_read_byte - internal function to read a single byte from SD
 */
uint16_t sd_read_byte(void) {
  /* wait for any previous transmission to be done */
  while((SPI_SR(SD_SPI) & SPI_SR_BSY)) {;}

  /* make sure the RX data flag is clear */
  if(SPI_SR(SD_SPI) & SPI_SR_RXNE) {
    spi_read(SD_SPI);
  }

  /* write all 1s out, generates the necessary clocks */
  spi_write(SD_SPI, 0xFF);

  /* wait until a whole byte has been clocked back in */
  while((SPI_SR(SD_SPI) & SPI_SR_RXNE) == 0) {;}

  /* return the received byte */
  return spi_read(SD_SPI);
}

/**
 *  sd_write_byte - internal function to write a single byte to SD
 */
void sd_write_byte(uint16_t dat) {
  /* make sure there are no previous transmissions on-going */
  while((SPI_SR(SD_SPI) & SPI_SR_TXE) == 0) {;}

  /* write the data */
  spi_write(SD_SPI, dat);
}

/**
 *  sd_command - internal function to send a properly formatted command to
 *               to the SD card.
 */
uint16_t sd_command(u8 code, u32 data, u8 chksm) {
  uint16_t c;

  if(code & 0x80) {
    /* it's an ACMD, so send CMD 55 first */
    sd_read_byte();
  
    sd_write_byte(0x40 + 55);
    sd_write_byte(0x00);
    sd_write_byte(0x00);
    sd_write_byte(0x00);
    sd_write_byte(0x00);
    sd_write_byte(0x01);

    do {
      c = sd_read_byte();
    } while(c == 0xFF);
  }

  sd_read_byte();

  sd_write_byte(0x40 + (code & 0x7F));
  sd_write_byte((data >> 24) & 0xFF);
  sd_write_byte((data >> 16) & 0xFF);
  sd_write_byte((data >> 8) & 0xFF);
  sd_write_byte((data & 0xFF));
  sd_write_byte(chksm);

  if(code == CMD12) {
    sd_read_byte();     /* for CMD12 we have to discard a byte */
  }

  do {
    c = sd_read_byte();
  } while(c == 0xFF);

  return c;
}

/**
 *  sd_init - public function that sets-up the IO and clocks
 */
uint8_t sd_init() {

  /* need to do the clocks */
  rcc_peripheral_enable_clock(&SD_SPI_APB, SD_RCC_SPI);
  rcc_peripheral_enable_clock(&SD_IO_APB, SD_RCC_IO);

  rcc_peripheral_enable_clock(&RCC_AHBENR, RCC_AHBENR_DMA1EN);

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
  spi_set_unidirectional_mode(SD_SPI); /* we want to send only */
  spi_disable_crc(SD_SPI); /* no CRC for this slave */
  spi_set_dff_8bit(SD_SPI); /* 8-bit dataword-length */
  spi_set_full_duplex_mode(SD_SPI);  /* not receive-only */
  spi_enable_software_slave_management(SD_SPI); /* we want to handle the CS signal in software */
  spi_set_nss_high(SD_SPI);
  spi_set_baudrate_prescaler(SD_SPI, SPI_CR1_BR_FPCLK_DIV_2); /* PCLOCK/256 as clock */
  spi_set_master_mode(SD_SPI); /* we want to control everything and generate the clock -> master */
  spi_set_clock_polarity_0(SD_SPI); /* sck idle state high */
  spi_set_clock_phase_0(SD_SPI); /* bit is taken on the second (rising edge) of sck */
  spi_disable_ss_output(SD_SPI);
  spi_enable(SD_SPI);

//   spi_read(SD_SPI);   /* make sure the new data flag isn't set */

  /* setup the interrupts for DMA transfers */
  nvic_enable_irq(NVIC_DMA1_CHANNEL4_IRQ);
  nvic_set_priority(NVIC_DMA1_CHANNEL4_IRQ, 1);

  return 0;
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
    sd_read_byte();
  }

  /* set chip select low again */
  gpio_clear(SD_PORT, SD_CS);

//  for(i=0;i<1000;i++);

  /* send CMD0 with the correct (pre-computed) CRC */
  c = sd_command(CMD0, 0, 0x95);
  //usart_dec_u16(c);
  //usart_puts("\n");

  /* send CMD8 to see if this is a mark 2 SD card */
  c = sd_command(CMD8, 0x000001AA, 0x87);   /* data pattern is used to check voltage levels */
  if(c == 5) {
    card.card_type = SD_CARD_SC;
  } else if(c == 1) {
    /* card is type 2 need to check for voltage/speed corruption */
    if(sd_read_byte() != 0) {
      card.card_type = SD_CARD_ERROR;
      return -1;// SD_CARD_ERROR;
    }
    if(sd_read_byte() != 0) {
      card.card_type = SD_CARD_ERROR;
      return -1;// SD_CARD_ERROR;
    }
    if(sd_read_byte() != 1) {
      card.card_type = SD_CARD_ERROR;
      return -1;// SD_CARD_ERROR;
    }
    if(sd_read_byte() != 0xAA) {
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
    c = sd_read_byte();
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
  c = sd_read_byte();
  /* contains the card type info in top 2 bits */
  /* also determines the structure of the rest of the CSD */
  if(c & 0x40) {
    card.card_type = SD_CARD_HC;
    sd_read_byte();   /* this is always 0x0E no need to check */
    sd_read_byte();   /* this is always 0x00 */
    sd_read_byte();   /* card speed 0x32 or 0x5A, we don't care */
    sd_read_byte();   /* card command class, don't care */
    sd_read_byte();   /* end of class and max block len, don't care */
    sd_read_byte();   /* DSR bit and zeros, don't care */
    c = sd_read_byte();   /* 4 zeros and top 4 bits of size */
    card.size = (c & 0xF) << 16;
    c = sd_read_byte();   /* next byte of size */
    card.size += (c << 8);
    c = sd_read_byte();
    card.size += c;      /* last byte of size */
    card.size <<= 10;    /* want it in 512 blocks but was in 512k */
    sd_read_byte();   /* always 0x7F */
    sd_read_byte();   /* always 0x80 */
    sd_read_byte();   /* always 0x0A */
    sd_read_byte();   /* always 0x40 */
    sd_read_byte();   /* write protect flags, we're ignoring */
    sd_read_byte();   /* checksum */
  } else {
    card.card_type = SD_CARD_SC;
    sd_read_byte();   /* read time, don't care */
    sd_read_byte();   /* more access time */
    sd_read_byte();   /* speed don't care */
    sd_read_byte();   /* ccc part 1 */
    c = sd_read_byte();   /* read block len */
    i = c & 0xF;
    c = sd_read_byte();   /* various flags and top 2 bits of C_SIZE */
    card.size = (c & 0x03) << 10;
    c = sd_read_byte();   /* middle 8 bits of C_SIZE */
    card.size += (c << 2);
    c = sd_read_byte();   /* last two bits and some current info */
    card.size += ((c & 0xC0) >> 6);
    c = sd_read_byte();   /* more current info and top 2 bits of size_mult */
    c = (c << 1) + (sd_read_byte() >> 7);
    c = 1 << ((c & 0x7) + 2);
    card.size++;
    card.size *= c;
    card.size <<= (i - 9);
    sd_read_byte();   /* write protect nonsense */
    sd_read_byte();   /* write info */
    sd_read_byte();   /* more of the same */
    sd_read_byte();   /* pre-pressed stuff and WP */
    sd_read_byte();   /* the checksum */
  }

  return 0;
}

/**
 *  sd_read_block - Get a 512 byte block from the SD card into buffer
 */
uint16_t sd_read_block(char *buffer, uint32_t addr) {
  int i;
  u16 c;
  if(card.card_type == SD_CARD_SC) {
    addr <<= 9;
  }

  c = sd_command(CMD17, addr, 1);

  if(c != 0) {
    return c;
  }
  
  do {
    c = sd_read_byte();
  } while(c != 0xFE);

  for(i=0;i<512;i++) {
    buffer[i] = sd_read_byte();
  }
  sd_read_byte();
  sd_read_byte();   /* read checksum bytes and dispose of */

  return 0;
}

/**
 *  sd_read_multiblock - use the multi block transfer and internal DMA
 *                       to fetch multiple 512 byte blocks at once
 **/
uint16_t sd_read_multiblock2(char *buffer, u32 addr, u8 num_blocks) {
  int i, j;
  u16 c;
  if(card.card_type == SD_CARD_SC) {
    addr <<= 9;
  }

  c = sd_command(CMD18, addr, 1); /* start multiblock mode */

  if(c != 0) {
    return 18;
  }

  for(j=0;j<num_blocks;j++) {     /* loop over number of blocks */
    do {
      c = sd_read_byte();         /* wait for a data token */
    } while(c != 0xFE);

    for(i=0;i<512;i++) {          /* read the actual data */
      buffer[j*512+i] = sd_read_byte();
    }
   sd_read_byte();               /* dump the two checksum bytes */
   sd_read_byte();
//     iprintf("%x\n", sd_read_byte());
//     iprintf("%x\n", sd_read_byte());
  }
  c = sd_command(CMD12, 0, 1);    /* end multiblock mode */
  return c;
}

/**
 *  sd_read_multiblock - use the multi block transfer and internal DMA
 *                       to fetch multiple 512 byte blocks at once
 **/
uint16_t sd_read_multiblock3(char *buffer, u32 addr, u8 num_blocks) {
  int i, j;
  uint16_t c;
  char *garbage = "\xFF";

  dma_set_priority(DMA1, DMA_CHANNEL4, DMA_CCR_PL_VERY_HIGH);
  dma_set_priority(DMA1, DMA_CHANNEL5, DMA_CCR_PL_LOW);
  dma_set_memory_size(DMA1, DMA_CHANNEL4, DMA_CCR_MSIZE_8BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL5, DMA_CCR_MSIZE_8BIT);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL4, DMA_CCR_PSIZE_8BIT);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL5, DMA_CCR_PSIZE_8BIT);

  /* setup dma */
  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL4);
//  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL5);
  DMA_CCR(DMA1, 5) &= ~ DMA_CCR_MINC;
//  dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL4);

  dma_set_read_from_peripheral(DMA1, DMA_CHANNEL4);
  dma_set_read_from_memory(DMA1, DMA_CHANNEL5);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL4, (u32)&SPI_DR(SD_SPI));
  dma_set_peripheral_address(DMA1, DMA_CHANNEL5, (u32)&SPI_DR(SD_SPI));

//  iprintf("garbage = %p\n", garbage);
//  iprintf("CMAR5 = %d\n", DMA_CMAR5(DMA1));
//  dma_set_memory_address(DMA1, DMA_CHANNEL4, (u32)buffer);
//  dma_set_memory_address(DMA1, DMA_CHANNEL5, (u32)garbage);
//  dma_enable_channel(DMA1, DMA_CHANNEL5);
//  spi_enable_rx_dma(SD_SPI);
//  spi_enable_tx_dma(SD_SPI);

//  iprintf("DMA_CCR4 %08X\n", DMA_CCR4(DMA1));
//  iprintf("DMA_CNDTR4 %08X\n", DMA_CNDTR4(DMA1));

  if(card.card_type == SD_CARD_SC) {
    addr <<= 9;
  }

  c = sd_command(CMD18, addr, 1); /* start multiblock mode */

  if(c != 0) {
    return 18;
  }

  for(j=0;j<num_blocks;j++) {     /* loop over number of blocks */
    do {
      c = sd_read_byte();         /* wait for a data token */
    } while(c != 0xFE);

//    iprintf("SPI_SR = %08X\n", SPI_SR(SD_SPI));
//    DMA_CNDTR4(DMA1) = 512;
    dma_set_number_of_data(DMA1, DMA_CHANNEL4, 512);
    dma_set_memory_address(DMA1, DMA_CHANNEL4, (u32)(buffer + j * 512));
//    dma_set_number_of_data(DMA1, DMA_CHANNEL4, 512);
    dma_set_number_of_data(DMA1, DMA_CHANNEL5, 512);
    dma_set_memory_address(DMA1, DMA_CHANNEL5, (u32)(garbage));
//    iprintf("starting DMA...\n");
//    spi_disable(SD_SPI);
//    spi_set_receive_only_mode(SD_SPI);
//    spi_enable(SD_SPI);
//    iprintf("DMA_ISR %08X\n", DMA_ISR(DMA1));
//    iprintf("DMA_CCR4 %08X\n", DMA_CCR4(DMA1));
//    iprintf("DMA_CNDTR4 %08X\n", DMA_CNDTR4(DMA1));
//    iprintf("DMA_CNDTR5 %08X\n", DMA_CNDTR5(DMA1));
//    iprintf("DMA_CMAR4 %08X\n", DMA_CMAR4(DMA1));
//    iprintf("DMA_CMAR5 %08X\n", DMA_CMAR5(DMA1));
    dma_enable_channel(DMA1, DMA_CHANNEL5);
    dma_enable_channel(DMA1, DMA_CHANNEL4);

    SPI_CR2(SD_SPI) |= (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
//    spi_set_receive_only_mode(SD_SPI);
    while(!(DMA_ISR(DMA1) & DMA_ISR_TCIF4)) {;}
//    iprintf("DMA_ISR(DMA1) = %08X\n", DMA_ISR(DMA1));
//    spi_set_full_duplex_mode(SD_SPI);
    DMA_IFCR(DMA1) |= DMA_IFCR_CTCIF4 | DMA_IFCR_CGIF4 | DMA_IFCR_CHTIF4;
    DMA_IFCR(DMA1) |= DMA_IFCR_CTCIF5 | DMA_IFCR_CGIF5 | DMA_IFCR_CHTIF5;
    dma_disable_channel(DMA1, DMA_CHANNEL4);
    dma_disable_channel(DMA1, DMA_CHANNEL5);
    SPI_CR2(SD_SPI) &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
//    iprintf("SPI_SR = %08X\n", SPI_SR(SD_SPI));
//    spi_disable(SD_SPI);
//    spi_set_full_duplex_mode(SD_SPI);
//    spi_enable(SD_SPI);
//    iprintf("finished DMA\n");

//    for(i=0;i<512;i++) {          /* read the actual data */
//      buffer[j*512+i] = sd_read_byte();
//    }
    sd_read_byte();
    sd_read_byte();
//    iprintf("%x\n", sd_read_byte());               /* dump the two checksum bytes */
//    iprintf("%x\n", sd_read_byte());
  }

//    dma_disable_channel(DMA1, DMA_CHANNEL4);
//    spi_disable_rx_dma(SD_SPI);
//    spi_disable_tx_dma(SD_SPI);

  c = sd_command(CMD12, 0, 1);    /* end multiblock mode */
  return c;
}

void start_dma_transfer() {
  uint8_t c;
  do {
    c = sd_read_byte();         /* wait for a data token */
  } while(c != 0xFE);

  dma_set_number_of_data(DMA1, DMA_CHANNEL4, 512);
  dma_set_memory_address(DMA1, DMA_CHANNEL4, (u32)(dma_buffer_addr + dma_block_no * 512));
  dma_set_number_of_data(DMA1, DMA_CHANNEL5, 512);
  dma_set_memory_address(DMA1, DMA_CHANNEL5, (u32)(garbage));
  dma_enable_channel(DMA1, DMA_CHANNEL5);
  dma_enable_channel(DMA1, DMA_CHANNEL4);

  SPI_CR2(SD_SPI) |= (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
  return;
}

void dma1_channel4_isr() {
  uint8_t c;

  gpio_toggle(GPIOC, GPIO12);

  DMA_IFCR(DMA1) |= DMA_IFCR_CTCIF4 | DMA_IFCR_CGIF4 | DMA_IFCR_CHTIF4;
  DMA_IFCR(DMA1) |= DMA_IFCR_CTCIF5 | DMA_IFCR_CGIF5 | DMA_IFCR_CHTIF5;
  dma_disable_channel(DMA1, DMA_CHANNEL4);
  dma_disable_channel(DMA1, DMA_CHANNEL5);
  SPI_CR2(SD_SPI) &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
  sd_read_byte();
  sd_read_byte();

//  dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL4);
//  return;

  if(++dma_block_no < dma_total_blocks) {
//    dma_buffer_addr += 512;
    start_dma_transfer();
  } else {
    /* mark dma transfer done */
    dma_state = 0;
    *dma_report_done = 0;
//    iprintf("%x\n", sd_read_byte());               /* dump the two checksum bytes */
//    iprintf("%x\n", sd_read_byte());
    c = sd_command(CMD12, 0, 1);    /* end multiblock mode */
  }
}

/**
 *  sd_read_multiblock - use the multi block transfer and internal DMA
 *                       to fetch multiple 512 byte blocks at once
 **/
uint16_t sd_read_multiblock(char *buffer, uint32_t addr, uint8_t num_blocks, unsigned int *flags) {
  int i, j;
  uint16_t c;

  /* block until previous transfer finishes if there's one going on */
  while(dma_state) {;}

  dma_set_priority(DMA1, DMA_CHANNEL4, DMA_CCR_PL_VERY_HIGH);
  dma_set_priority(DMA1, DMA_CHANNEL5, DMA_CCR_PL_LOW);
  dma_set_memory_size(DMA1, DMA_CHANNEL4, DMA_CCR_MSIZE_8BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL5, DMA_CCR_MSIZE_8BIT);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL4, DMA_CCR_PSIZE_8BIT);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL5, DMA_CCR_PSIZE_8BIT);

  /* setup dma */
  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL4);
//  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL5);
  DMA_CCR(DMA1, 5) &= ~ DMA_CCR_MINC;
//  dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL4);

  dma_set_read_from_peripheral(DMA1, DMA_CHANNEL4);
  dma_set_read_from_memory(DMA1, DMA_CHANNEL5);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL4, (u32)&SPI_DR(SD_SPI));
  dma_set_peripheral_address(DMA1, DMA_CHANNEL5, (u32)&SPI_DR(SD_SPI));

  /* enable the interrupt */
  dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL4);

//  iprintf("garbage = %p\n", garbage);
//  iprintf("CMAR5 = %d\n", DMA_CMAR5(DMA1));
//  dma_set_memory_address(DMA1, DMA_CHANNEL4, (u32)buffer);
//  dma_set_memory_address(DMA1, DMA_CHANNEL5, (u32)garbage);
//  dma_enable_channel(DMA1, DMA_CHANNEL5);
//  spi_enable_rx_dma(SD_SPI);
//  spi_enable_tx_dma(SD_SPI);

//  iprintf("DMA_CCR4 %08X\n", DMA_CCR4(DMA1));
//  iprintf("DMA_CNDTR4 %08X\n", DMA_CNDTR4(DMA1));

  if(card.card_type == SD_CARD_SC) {
    addr <<= 9;
  }

  c = sd_command(CMD18, addr, 1); /* start multiblock mode */

  if(c != 0) {
    return c;
  }

  dma_buffer_addr = buffer;
  dma_block_no = 0;
  dma_total_blocks = num_blocks;
  dma_state = 1;
  dma_report_done = flags;
  //for(j=0;j<num_blocks;j++) {     /* loop over number of blocks */
  //  dma_block_no = j;
  start_dma_transfer();
//  while(dma_state) {
//    iprintf("DMA block %d\n", dma_block_no);
//    while(!(DMA_ISR(DMA1) & DMA_ISR_TCIF4)) {;}
//    dma_transfer_done();
//  }
  return;
}

uint8_t sd_find_partition() {
  char buffer[512];
  uint8_t i;
  mbr_entry *ent;

  //iprintf("sd_find_partiton; loading block\n");

  sd_read_block(buffer, 0);   /* read the MBR into the buffer */

  //iprintf("sd_find_partition; loaded block\n");

  ent = (mbr_entry *)(buffer + PARTITION0START);

  //iprintf("sd_find_partition; part0 type %02X\n", ent->type);

  for(i=0;i<4;i++) {
#ifdef SUPPORT_FAT16
    if(ent->type == PART_TYPE_FAT16) {
      break;
    }
#endif
#ifdef SUPPORT_FAT32
    if(ent->type == PART_TYPE_FAT32) {
      break;
    }
#endif
    ent = (mbr_entry *)(ent + 16);
  }

  //iprintf("sd_find_partition; i reached %d\n", i);
  if(i < 4) {
    //iprintf("sd_find_partition; partition type %02X\n", ent->type);
    card.fs = ent->type;
    card.part = ent->lba_start;
    card.part_size = ent->length;
    return 0;
  } else {
    card.fs = 0;
    card.error = SD_ERR_NO_PART;
    return -1;
  }
}

