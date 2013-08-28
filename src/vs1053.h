#ifndef VS1053_H
#define VS1053_H 1

#include <stdint.h>

#define CODEC_PORT      GPIOA
#define CODEC_CS        GPIO4
#define CODEC_SCK       GPIO5
#define CODEC_MISO      GPIO6
#define CODEC_MOSI      GPIO7

#define CODEC_DREQ_PORT GPIOA
#define CODEC_DREQ      GPIO8
#define CODEC_RST_PORT  GPIOA
#define CODEC_RST       GPIO3
#define CODEC_PWR_PORT  GPIOB
#define CODEC_PWR       GPIO5

#define CODEC_IOS_APB   RCC_APB2ENR
#define CODEC_RCC_IOS   RCC_APB2ENR_IOPAEN

#define CODEC_PWR_APB   RCC_APB2ENR
#define CODEC_RCC_PWR   RCC_APB2ENR_IOPBEN

#define CODEC_IOI_APB   RCC_APB2ENR
#define CODEC_RCC_IOI   RCC_APB2ENR_IOPCEN

#define CODEC_SPI       SPI1
#define CODEC_SPI_APB   RCC_APB2ENR
#define CODEC_RCC_SPI   RCC_APB2ENR_SPI1EN

/* address constants on the co-processor */

#define SCI_MODE        0x0
#define SCI_STATUS      0x1
#define SCI_BASS        0x2
#define SCI_CLOCKF      0x3
#define SCI_DECODE_TIME 0x4
#define SCI_AUDATA      0x5
#define SCI_WRAM        0x6
#define SCI_WRAMADDR    0x7
#define SCI_HDAT0       0x8
#define SCI_HDAT1       0x9
#define SCI_AIADDR      0xA
#define SCI_VOL         0xB
#define SCI_AICTRL0     0xC
#define SCI_AICTRL1     0xD
#define SCI_AICTRL2     0xE
#define SCI_AICTRL3     0xF

/* bit masks for the SCI_MODE register */
#define SM_DIFF           0x0001
#define SM_LAYER12        0x0002
#define SM_RESET          0x0004
#define SM_CANCEL         0x0008
#define SM_EARSPEAKER_LO  0x0010
#define SM_TESTS          0x0020
#define SM_STREAM         0x0040
#define SM_EARSPEAKER_HI  0x0080
#define SM_DACT           0x0100
#define SM_SDIORD         0x0200
#define SM_SDISHARE       0x0400
#define SM_SDINEW         0x0800
#define SM_ADPCM          0x1000
/* unused */
#define SM_LINE1          0x4000
#define SM_CLK_RANGE      0x8000

#define PARAM_END_FILL_BYTE 0x1E06    /* address of fill byte in X RAM */
#define PARAM_POSITION_LO   0x1e27
#define PARAM_POSITION_HI   0x1e28    /* Ogg file playback position */

struct player_status {
  int byte_count;
  int playing;
  uint32_t pos;
};

void init_codec();
void demo_codec();
void play_file(char *);

void play_file_fast_async(char *);

//sstatic void player_task(void *);
#endif

