#ifndef SD_H
#define SD_H 1

#include <stdint.h>

/**
 *  Variables that need to be changed on different hardware
 */

/* Pin definitions for portability */
#define SD_PORT       GPIOB
#define SD_CS         GPIO12
#define SD_SCK        GPIO13
#define SD_MISO       GPIO14
#define SD_MOSI       GPIO15

/* optional definitions for the Write Protect and Card Present pins */
#define SD_CP         GPIO13
#define SD_CP_PORT    GPIOC

#define SD_WP         GPIO11
#define SD_WP_PORT    GPIOB

#define SD_IO_APB    RCC_APB2ENR
#define SD_RCC_IO    RCC_APB2ENR_IOPBEN

/* SPI hardware number */
#define SD_SPI        SPI2
#define SD_SPI_APB    RCC_APB1ENR
#define SD_RCC_SPI    RCC_APB1ENR_SPI2EN

/**
 *  Platform independent definitions
 */

/* Constants used to define card types */
#define SD_CARD_NONE  0    /* No SD card found */
#define SD_CARD_MMC   1    /* MMC card */
#define SD_CARD_SC    2    /* Standard capacity SD card (up to 2GB)*/
#define SD_CARD_HC    3    /* High capacity SD card (4GB to 32GB)*/
#define SD_CARD_XC    4    /* eXtended Capacity SD card (up to 2TB  - Untested may work if FAT32) */
#define SD_CARD_ERROR 99   /* An error occured during setup */

/* SPI commands */
#define CMD0          0
#define CMD1          1
#define CMD8          8
#define CMD9          9
#define CMD10         10
#define CMD12         12
#define CMD17         17
#define CMD18         18
#define ACMD41        0x80 + 41

/* Error status codes returned in the SD info struct */
#define SD_ERR_NO_PART      1
#define SD_ERR_NOT_PRESENT  2
#define SD_ERR_NO_FAT       3

/* SD card info struct */
typedef struct {
  uint16_t  card_type;
  uint8_t   read_only;
  uint32_t  size;
  uint8_t   fs;
  uint32_t  part;
  uint32_t  part_size;
  uint8_t   error;
  uint8_t   fat_entry_len;
  uint32_t  end_cluster_marker;
  uint8_t   sectors_per_cluster;
  uint32_t  cluster0;
  uint32_t  active_fat_start;
  uint32_t  root_len;
  uint32_t  root_start;
  uint32_t  root_cluster;
} SDCard;

struct dma_job {
  uint32_t  block;
  uint32_t  count;
  char *buffer;
  volatile uint32_t *flags;
};

uint8_t sd_init();
uint8_t sd_card_reset();
//u16 spi_rw(u32, u16);
//void sd_cid(void);
//u16 sd_command(u8, u32, u8);
uint16_t sd_read_block(char *, uint32_t);
void sd_read_multiblock(char *, uint32_t, uint8_t, volatile uint32_t *);
uint8_t sd_find_partition();

#endif

