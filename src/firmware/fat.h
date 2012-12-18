/*
 * This file is part of the oggbox project.
 *
 * Copyright Nathan Dumont 2012 <nathan@nathandumont.com>
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

#ifndef FAT_H
#define FAT_H 1

#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include "block.h"
#include "dirent.h"

#define MAX_OPEN_FILES 4

/* #define STDIN_FILENO 0 */ /* standard input file descriptor */
/* #define STDOUT_FILENO 1 */ /* standard output file descriptor */
/* #define STDERR_FILENO 2 */ /* standard error file descriptor */
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

struct fat_info {
  uint8_t   read_only;
  uint8_t   fat_entry_len;
  uint32_t  end_cluster_marker;
  uint8_t   sectors_per_cluster;
  uint32_t  cluster0;
  uint32_t  active_fat_start;
  uint32_t  root_len;
  uint32_t  root_start;
  uint32_t  root_cluster;
  uint8_t   type;               // type of filesystem (FAT16 or FAT32)
  uint8_t   part_start;         // start of partition containing filesystem
};

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
  uint8_t   fs_dirty;
  char      filename[8];
  char      extension[3];
  uint8_t   attributes;
//   uint8_t   reserved;
//   uint8_t   create_time_fine;
//   uint16_t  create_time;
//   uint16_t  create_date;
//   uint16_t  access_date;
//   uint16_t  high_first_cluster;
//   uint16_t  modified_time;
//   uint16_t  modified_date;
//   uint16_t  first_cluster;
  uint32_t  size;
  uint32_t  full_first_cluster;
  uint32_t  entry_sector;
  uint8_t   entry_number;
  uint32_t  file_sector;
  uint32_t  append_mode;
  time_t    created;
  time_t    modified;
  time_t    accessed;
} FileS;

// #define MEDIA_BUFFER_SIZE 2048
// 
// typedef struct {
//   char buffer[2][MEDIA_BUFFER_SIZE];
//   char meta_buffer[512];
//   int  meta_block;
//   int  cluster;
//   unsigned char active_buffer;
//   int  block;
//   int  block_count;
//   int  error;
//   uint32_t buffer_ready[2];
//   unsigned char near_end;
//   unsigned char nearly_near_end;
//   int  file_end;
//   unsigned int file_len;
// } MediaFileS;

int fat_mount(blockno_t, uint8_t);

// int sdfat_init();
// int sdfat_mount();
int sdfat_lookup_path(int, const char *);
int fat_open(const char *, int);
int fat_close(int);
int sdfat_read(int, void *, int);
int sdfat_lseek(int, int, int);
int sdfat_get_next_dirent(int, struct dirent *);
int sdfat_stat(int fd, struct stat *st);

int sdfat_next_sector(int fd);

// char *sdfat_open_media(char *);
// char *sdfat_read_media();

#endif
