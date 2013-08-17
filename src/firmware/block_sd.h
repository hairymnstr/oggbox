#ifndef BLOCK_SD_H
#define BLOCK_SD_H 1

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

#define SD_RETRIES 1000

/* SD card info struct */
typedef struct {
  uint16_t  card_type;
  uint8_t   read_only;
  uint32_t  size;
  uint8_t   error;
} SDCard;

#endif /* ifndef BLOCK_SD_H */
