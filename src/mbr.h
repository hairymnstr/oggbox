#ifndef MBR_H
#define MBR_H 1

#include <stdint.h>

#define PARTITION0START 0x1BE
#define PARTITION1START 0x1CE
#define PARTITION2START 0x1DE
#define PARTITION3START 0x1EE

#define PART_TYPE_FAT16 0x06
#define PART_TYPE_FAT32 0x0B

typedef struct {
  uint8_t  bootable;
  uint8_t  chs[3];
  uint8_t  type;
  uint8_t  chs_end[3];
  uint32_t lba_start;
  uint32_t length;
} __attribute__((__packed__)) mbr_entry;

#endif
   
