#ifndef SDFAT_H
#define SDFAT_H 1

#include <stdint.h>
#include <sys/stat.h>
// #include <libopenstm32/common.h>
#include "dirent.h"

#define MAX_OPEN_FILES 4

#define STDIN_FILENO 0  /* standard input file descriptor */
#define STDOUT_FILENO 1 /* standard output file descriptor */
#define STDERR_FILENO 2 /* standard error file descriptor */
#define FIRST_DISC_FILENO 3 /* first real file (on disc) */

#define FAT_ERROR_CLUSTER 1
#define FAT_END_OF_FILE 2

/* FAT file attribute bit masks */
#define FAT_ATT_RO  0x01
#define FAT_ATT_HID 0x02
#define FAT_ATT_SYS 0x04
#define FAT_ATT_VOL 0x08
#define FAT_ATT_SUBDIR 0x10
#define FAT_ATT_ARC 0x20
#define FAT_ATT_DEV 0x40


typedef struct {
  uint8_t       jump[3];
  char          name[8];
  uint16_t      sector_size;
  uint8_t       cluster_size;
  uint16_t      reserved_sectors;
  uint8_t       num_fats;
  uint16_t      root_entries;
  uint16_t      total_sectors;
  uint8_t       media_descriptor;
  uint16_t      sectors_per_fat;
  uint16_t      sectors_per_track;
  uint16_t  number_of_heads;
  uint32_t  partition_start;
  uint32_t  big_total_sectors;
  uint8_t   drive_number;
  uint8_t   current_head;
  uint8_t   boot_sig;
  uint32_t  volume_id;
  char volume_label[11];
  char fs_label[8];
} __attribute__((__packed__)) boot_sector_fat16;

typedef struct {
  uint8_t   jump[3];                   /*    0 */
  char name[8];                   /*    3 */
  uint16_t  sector_size;               /*    B */
  uint8_t   cluster_size;              /*    D */
  uint16_t  reserved_sectors;          /*    E */
  uint8_t   num_fats;                  /*   10 */
  uint16_t  root_entries;              /*   11 */
  uint16_t  total_sectors;             /*   13 */
  uint8_t   media_descriptor;          /*   15 */
  uint16_t  short_sectors_per_fat;     /*   16 */
  uint16_t  sectors_per_track;         /*   18 */
  uint16_t  number_of_heads;           /*   1A */
  uint32_t  partition_start;           /*   1C */
  uint32_t  big_total_sectors;         /*   20 */
  uint32_t  sectors_per_fat;           /*   24 */
  uint16_t  fat_flags;                 /*   28 */
  uint16_t  version;                   /*   2A */
  uint32_t  root_start;                /*   2C */
  uint16_t  fs_info_start;             /*   30 */
  uint16_t  boot_copy;                 /*   32 */
  char reserved[12];              /*   34 */
  uint8_t   drive_number;              /*   40 */
  uint8_t   current_head;              /*   41 */
  uint8_t   boot_sig;                  /*   42 */
  uint32_t  volume_id;                 /*   43 */
  char volume_label[11];          /*   47 */
  char fs_label[8];               /*   52 */
} __attribute__((__packed__)) boot_sector_fat32;

#define FS_INFO_SIG1 0x0000
#define FS_INFO_SIG2 0x01E4
#define FREE_CLUSTERS 0x01E8
#define LAST_ALLOCATED 0x01EC

typedef struct {
  char filename[8];
  char extension[3];
  uint8_t   attributes;
  uint8_t   reserved;
  uint8_t   create_time_fine;
  uint16_t  create_time;
  uint16_t  create_date;
  uint16_t  access_date;
  uint16_t  high_first_cluster;
  uint16_t  modified_time;
  uint16_t  modified_date;
  uint16_t  first_cluster;
  uint32_t  size;
} __attribute__((__packed__)) direntS;

typedef struct {
  char buffer[512];
  uint32_t  sector;
  uint32_t  cluster;
  uint8_t   sectors_left;
  uint16_t  cursor;
  uint8_t   error;
  uint8_t   dirty;
  char filename[8];
  char extension[3];
  uint8_t   attributes;
  uint8_t   reserved;
  uint8_t   create_time_fine;
  uint16_t  create_time;
  uint16_t  create_date;
  uint16_t  access_date;
  uint16_t  high_first_cluster;
  uint16_t  modified_time;
  uint16_t  modified_date;
  uint16_t  first_cluster;
  uint32_t  size;
  uint32_t  full_first_cluster;
  uint32_t  entry_sector;
  uint8_t   entry_number;
  uint32_t  file_sector;
} __attribute__((__packed__)) FileS;

typedef struct {
  char buffer[2][2048];
  char meta_buffer[512];
  int  meta_block;
  int  cluster;
  unsigned char active_buffer;
  int  block;
  int  block_count;
  int  error;
  unsigned int buffer_ready[2];
  unsigned char near_end;
  unsigned char nearly_near_end;
  int  file_end;
  unsigned int file_len;
} MediaFileS;

int sdfat_init();
int sdfat_mount();
int sdfat_lookup_path(int, const char *);
int sdfat_open(const char *, int);
int sdfat_read(int, void *, int);
int sdfat_get_next_dirent(int, struct dirent *);
int sdfat_stat(int fd, struct stat *st);
#endif

