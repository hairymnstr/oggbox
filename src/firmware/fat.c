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

// #include <libopencm3/stm32/nvic.h>

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "dirent.h"
#include "block.h"
// #include "sdfat.h"
#include "mbr.h"
// #include "sd.h"

/**
 * global variable structures.
 * These take the place of a real operating system.
 **/

SDCard card;
FileS file_num[MAX_OPEN_FILES];
uint32_t available_files;
volatile MediaFileS media_file;

extern int errno;

/**
 * low level FAT access routines
 **/

/* sdfat_to_unix_time - convert a time field from FAT format to unix epoch 
   seconds. */
time_t fat_to_unix_time(uint16_t time) {
  struct tm time_str;
  time_str.tm_year = 0;
  time_str.tm_mon = 0;
  time_str.tm_mday = 0;
  time_str.tm_hour = ((time & 0xF800) >> 11);
  time_str.tm_min = ((time & 0x03E0) >> 5);
  time_str.tm_sec = (time & 0x001F) << 1;
  time_str.tm_isdst = -1;
  return mktime(&time_str);
}

//uint16_t sdfat_from_unix_time(int seconds) {
//
//}

time_t fat_to_unix_date(uint16_t date) {
  struct tm time_str;

  time_str.tm_year = (((date & 0xFE00) >> 9) + 80);
  time_str.tm_mon = (((date & 0x01E0) >> 5) - 1);
  time_str.tm_mday = (date & 0x001F) ;
  time_str.tm_hour = 0;
  time_str.tm_min = 0;
  time_str.tm_sec = 0;
  time_str.tm_isdst = -1;

  return mktime(&time_str);
}

/* sdfat_get_next_file - returns the next free file descriptor or -1 if none */
int8_t fat_get_next_file() {
  uint32_t i = 1;
  int j;

  for(j=0;j<MAX_OPEN_FILES;j++) {
    if((available_files & i) == 0) {
      available_files |= i;
      return j;
    }
    i <<= 1;
  }
  return -1;
}

int fat_mount(blockno_t part_start, uint8_t filesystem) {
  uint8_t buffer[512];
  boot_sector_fat16 *boot16;
  boot_sector_fat32 *boot32;
  
  block_read(part_start, (void *)buffer);
  if(filesystem == PART_TYPE_FAT16) {
    fatfs.fat_entry_len = 2;
    fatfs.end_cluster_marker = 0xFFF0;
    boot16 = (boot_sector_fat16 *)buffer;
    fatfs.sectors_per_cluster = boot16->cluster_size;
    fatfs.root_len = boot16->root_entries;
    i = part_start;
    i += boot16->reserved_sectors;
    fatfs.active_fat_start = i;
    i += (boot16->sectors_per_fat * boot16->num_fats);
    fatfs.root_start = i;
    i += (boot16->root_entries * 32) / 512;
    i -= (boot16->cluster_size * 2);
    fatfs.cluster0 = i;
    fatfs.root_cluster = 1;
    fatfs.type = PART_TYPE_FAT16;
    fatfs.part_start = part_start;
  } else if(filesystem == PART_TYPE_FAT32) {
    fatfs.fat_entry_len = 4;
    fatfs.end_cluster_marker = 0xFFFFFFF0;
    boot32 = (boot_sector_fat32 *)buffer;
    fatfs.sectors_per_cluster = boot32->cluster_size;
    i = part_start;
    i += boot32->reserved_sectors;
    fatfs.active_fat_start = i;
    i += boot32->sectors_per_fat * boot32->num_fats;
    i -= boot32->cluster_size * 2;
    fatfs.cluster0 = i;
    fatfs.root_cluster = boot32->root_start;
  } else {
    return -1;
  }
  
  return 0;
}

uint8_t sd_mount_part() {
  uint32_t i;
  boot_sector_fat16 *boot16;
  boot_sector_fat32 *boot32;

  if(!((card.fs == PART_TYPE_FAT16) || (card.fs == PART_TYPE_FAT32))) {
    card.error = SD_ERR_NO_FAT;
    return -1;
  }
  sd_read_block(file_num[0].buffer, card.part);
  
  available_files = 0;
  if(card.fs == PART_TYPE_FAT16) {
    card.fat_entry_len = 2;
    card.end_cluster_marker = 0xFFF0;
    boot16 = (boot_sector_fat16 *)file_num[0].buffer;
    card.sectors_per_cluster = boot16->cluster_size;
    card.root_len = boot16->root_entries;
    i = card.part;
    i += boot16->reserved_sectors;
    card.active_fat_start = i;
    i += (boot16->sectors_per_fat * boot16->num_fats);
    card.root_start = i;
    i += (boot16->root_entries * 32) / 512;
    i -= (boot16->cluster_size * 2);
    card.cluster0 = i;
    card.root_cluster = 1;
  } else {
    card.fat_entry_len = 4;
    card.end_cluster_marker = 0xFFFFFFF0;
    boot32 = (boot_sector_fat32 *)(file_num[0].buffer);
    card.sectors_per_cluster = boot32->cluster_size;
    i = card.part;
    i += boot32->reserved_sectors;
    card.active_fat_start = i;
    i += (boot32->sectors_per_fat * boot32->num_fats);
    i -= (boot32->cluster_size * 2);
    card.cluster0 = i;
    card.root_cluster = boot32->root_start;
  }
  return 0;
}

/*
  doschar - returns a dos file entry compatible version of character c
            0 indicates c was 0 (i.e. end of string)
            1 indicates an illegal character
            / indicates a path separator (either / or \  is accepted)
            . indicates a literal .
            all other valid characters returned, lower case are capitalised. */
char doschar(char c) {
  if(c == 0) {
    return 0;
  } else if((c == '/') || (c =='\\')) {
    return '/';
  } else if(c == '.') {
    return '.';
  } else if((c >= 'A') && (c <= 'Z')) {
    return c;
  } else if((c >= '0') && (c <= '9')) {
    return c;
  } else if((c >= 'a') && (c <= 'z')) {
    return (c - 'a') + 'A';
  } else if(c == 0xE5) {
    return 0x05;
  } else if(c > 127) {
    return c;
  } else if((c == '!') || (c == '#') || (c == '$') || (c == '%') ||
            (c == '&') || (c == '\'') || (c == '(') || (c == ')') ||
            (c == '-') || (c == '@') || (c == '^') || (c == '_') ||
            (c == '`') || (c == '{') || (c == '}') || (c == '~') ||
            (c == ' ')) {
    return c;
  } else {
    return 1;
  }
}

int make_dos_name(char *dosname, const char *path, int *path_pointer) {
  int i;
  char c, ext_follows;

  iprintf("path input = %s\n", path);

  dosname[11] = 0;
  c = doschar(*(path + (*path_pointer)++));
  for(i=0;i<8;i++) {
    if((c == '/') || (c == 0)) {
      *(dosname + i) = ' ';
    } else if(c == '.') {
      if(i==0) {
        *(dosname + i) = '.';
        c = doschar(*(path + (*path_pointer)++));
      } else if(i==1) {
        if((path[*path_pointer] == 0) || (doschar(path[*path_pointer]) == '/')) {
          *(dosname + i) = '.';
          c = doschar(*(path + (*path_pointer)++));
        }
      } else {
        *(dosname + i) = ' ';
      }
    } else if(c == 1) {
      iprintf("Exit 1\n");
      return -1;
    } else {
      *(dosname + i) = c;
      c = doschar(*(path + (*path_pointer)++));
    }
  }
  iprintf("main exit char = %c (%x)\n", c, c);
  if(c == '.') {
    ext_follows = 1;
    c = doschar(*(path + (*path_pointer)++));
  } else if((c == '/') || (c == 0)) {
    ext_follows = 0;
  } else {
    c = doschar(*(path + (*path_pointer)++));
    if(c == '.') {
      ext_follows = 1;
      c = doschar(*(path + (*path_pointer)++));
    } else if((c == '/') || (c == 0)) {
      ext_follows = 0;
    } else {
      iprintf("Exit 2\n");
      return -1;      /* either an illegal character or a filename too long */
    }
  }
  for(i=0;i<3;i++) {
    if(ext_follows) {
      if((c == '/') || (c == 0)) {
        *(dosname + 8 + i) = ' ';
      } else if(c == 1) {
        return -1;    /* illegal character */
      } else if(c == '.') {
        return -1;
      } else {
        *(dosname + 8 + i) = c;
        c = doschar(*(path + (*path_pointer)++));
      }
    } else {
      *(dosname + 8 + i) = ' ';
    }
  }
  /* because we post increment path_pointer, it is now pointing at the next character, need to move back one */
  (*path_pointer)--;
  iprintf("dosname = %s, last char = %c (%x)\n", dosname, *(path + (*path_pointer)), *(path + (*path_pointer)));
  if((c == '/') || (c == 0)) {
    return 0; /* extension ends the filename. */
  } else {
    iprintf("Exit 3\n");
    return -1;  /* the extension is too long */
  }
}

/* strips out any padding spaces and adds a dot if there is an extension. */
int fatname_to_str(char *output, char *input) {
  int i;
  char *cpo=output;
  char *cpi=input;
  for(i=0;i<8;i++) {
    if(*cpi != ' ') {
      *cpo++ = *cpi++;
    } else {
      cpi++;
    }
  }
  if(*cpi == ' ') {
    *cpo = 0;
    return 0;
  }
  /* otherwise there is an extension of at least one character.
     so add a dot and carry on */
  *cpo++ = '.';
  for(i=0;i<3;i++) {
    if(*cpi == ' ') {
      break;
    }
    *cpo++ = *cpi++;
  }
  *cpo = 0;   /* null -terminate */
  return 0;   /* and return */
}

/* write a sector back to disc */
int sdfat_flush(int fd) {
  /* writing not yet implemented */
  /* just clear this flag so this isn't called every time from now on */
  file_num[fd].dirty = 0;
  return 0;
}

/* get the first sector of a given cluster */
int sdfat_select_cluster(int fd, uint32_t cluster) {

  file_num[fd].sector = cluster * card.sectors_per_cluster + card.cluster0;
  file_num[fd].sectors_left = card.sectors_per_cluster - 1;
  file_num[fd].cluster = cluster;
  file_num[fd].cursor = 0;

  return sd_read_block(file_num[fd].buffer, file_num[fd].sector);
}

/* get the next cluster in the current file */
int sdfat_next_cluster(int fd) {
  uint32_t i;
  uint32_t j;

  if(file_num[fd].dirty) {
    sdfat_flush(fd);
  }
  i = file_num[fd].cluster;
  i = i * card.fat_entry_len;     /* either 2 bytes for FAT16 or 4 for FAT32 */
  j = (i / 512) + card.active_fat_start; /* get the sector number we want */
  //iprintf("Reading sector %d of FAT at %d for cluster %d\n", (i / 512), card.active_fat_start + (i / 512), file_num[fd].cluster);
  if(sd_read_block(file_num[fd].buffer, j)) {
    return -1;
  }
  i = i & 0x1FF;
  j = file_num[fd].buffer[i++];
  j += (file_num[fd].buffer[i++] << 8);
  if(card.fs == PART_TYPE_FAT32) {
    j += file_num[fd].buffer[i++] << 16;
    j += file_num[fd].buffer[i++] << 24;
  }
  if(j < 2) {
    file_num[fd].error = FAT_ERROR_CLUSTER;
    iprintf("ERROR CLUSTER %08X\n", (unsigned int)j);
    return -1;
  } else if(j >= card.end_cluster_marker) {
    iprintf("END OF FILE %08X\n", (unsigned int)j);
    file_num[fd].error = FAT_END_OF_FILE;
    return -1;
  }
  return j;
}

/* get the next sector in the current file. */
int sdfat_next_sector(int fd) {
  /* if the current sector was written write to disc */
  if(file_num[fd].dirty) {
    sdfat_flush(fd);
  }
  /* see if we need another cluster */
  if(file_num[fd].sectors_left > 0) {
    file_num[fd].sectors_left--;
    file_num[fd].file_sector++;
    file_num[fd].cursor = 0;
    return sd_read_block(file_num[fd].buffer, ++file_num[fd].sector);
  } else {
    file_num[fd].file_sector++;
    return sdfat_select_cluster(fd, sdfat_next_cluster(fd));
  }
}

/**
 * externally callable setup routines
 **/

int sdfat_init() {
  available_files = 0;              
  sd_init();                        /* setup hardware for the SD Card */
  return 0;
}

int sdfat_mount() {
  if(sd_card_reset() != 0) {
    return -1;
  }
  if(sd_find_partition() != 0) {
    return -2;
  }
  if(sd_mount_part() != 0) {
    return -3;
  }
  return 0;
}

/**
 * callable file access routines
 */

int sdfat_open(const char *name, int mode) {
  int i;
  int8_t fd;

  iprintf("%s\r\n", name);
  fd = sdfat_get_next_file();
  iprintf("fd = %d\r\n", fd);
  iprintf("mode = %X\r\n", mode);
  if(fd < 0) {
    return -EMFILE;   /* too many open files */
  }
  i = sdfat_lookup_path(fd, name);
  if(i != 0) {
    /* file doesn't exist */
    if((mode & O_RDONLY) != 0) {
      /* tried to open a non-existent file for read */
      return -ENOENT;
    } else {
      /* opening a new file for writing */
      /* TODO */
      return -1;
    }
  } else {
    /* file does exist */
    if(mode == O_RDONLY) {
      /* read existing file */
      file_num[fd].file_sector = 0;
      iprintf("returning %d\r\n", fd);
      return fd;
    } else {
      iprintf("not just read only\r\n");
      if(mode & O_APPEND) {
        /* need to seek to the end */
        /* TODO */
        return -1;
      } else {
        file_num[fd].file_sector = 0;
        /* over-write the file */
        //return sdfat_open(i);
        return fd;
      }
    }
  }
}

int sdfat_close(int fn) {
  iprintf("sdfat_close %d\r\n", fn);
  if(!(available_files & (1 << fn))) {
    return -1;    /* tried to close a file that's not open */
  }
  available_files &= ~(1 << fn);
  return 0;
}

int sdfat_lookup_path(int fd, const char *path) {
  char dosname[12];
//   char *cp = (char *)path;
//   char c;
  char isdir;
  int i;
  int path_pointer = 0;
  direntS *de;
//   iprintf("sdfat_lookup_path\r\n");

  if(path[0] != '/') {
    return -2;                                /* bad path, we have no cwd */
  }

  /* select root directory */
//   iprintf("selecting card root dir on cluster %d\r\n", card.root_cluster);
  sdfat_select_cluster(fd, card.root_cluster);

  path_pointer++;

  if(*(path + path_pointer) == 0) {
    /* user selected the root directory to open. */
    file_num[fd].full_first_cluster = card.root_cluster;
    file_num[fd].entry_sector = 0;
    file_num[fd].entry_number = 0;
    file_num[fd].file_sector = 0;
    file_num[fd].attributes = FAT_ATT_SUBDIR;
    file_num[fd].size = 4096;
    file_num[fd].access_date = 0;
    file_num[fd].modified_date = 0;
    file_num[fd].modified_time = 0;
    file_num[fd].create_date = 0;
    file_num[fd].create_time = 0;
    sdfat_select_cluster(fd, file_num[fd].full_first_cluster);
    return 0;
  }

  while(1) {
    if(make_dos_name(dosname, path, &path_pointer)) {
      return -1;  /* invalid path name */
    }
    iprintf("%s\r\n", dosname);
    while(1) {
      iprintf("looping\r\n");
      for(i=0;i<16;i++) {
        if(strncmp(dosname, (char *)(file_num[fd].buffer + (i * 32)), 11) == 0) {
          break;
        }
        file_num[fd].buffer[i * 32 + 11] = 0;
        iprintf("%s %d\r\n", (char *)(file_num[fd].buffer + (i * 32)), i);
      }
      if(i == 16) {
        if(sdfat_next_sector(fd) != 0) {
          return -1;
        }
      } else {
        break;
      }
    }
    iprintf("got here %d\r\n", i);
    de = (direntS *)(file_num[fd].buffer + (i * 32));
    iprintf("%s\r\n", de->filename);
    isdir = de->attributes & 0x10;
    /* if dir, and there are more path elements, select */
    if(isdir && (doschar(path[path_pointer]) == '/') && (doschar(path[path_pointer + 1]) != 0)) {
      path_pointer++;
      if(card.fs == PART_TYPE_FAT16) {
        if(de->first_cluster == 0) {
          sdfat_select_cluster(fd, card.root_cluster);
        } else {
          sdfat_select_cluster(fd, de->first_cluster);
        }
      } else {
        if(de->first_cluster + (de->high_first_cluster << 16) == 0) {
          sdfat_select_cluster(fd, card.root_cluster);
        } else {
          sdfat_select_cluster(fd, de->first_cluster + (de->high_first_cluster << 16));
        }
      }
    } else {
      /* otherwise, setup the fd */
      memcpy((void *)(file_num[fd].filename), (void *)(file_num[fd].buffer + (i * 32)), 32);
      if(card.fs == PART_TYPE_FAT16) {
        file_num[fd].full_first_cluster = file_num[fd].first_cluster;
      } else {
        file_num[fd].full_first_cluster = file_num[fd].first_cluster + file_num[fd].high_first_cluster;
      }

      /* this following special case occurs when a subdirectory's .. entry is opened. */
      if(file_num[fd].full_first_cluster == 0) {
        file_num[fd].full_first_cluster = card.root_cluster;
      }

      file_num[fd].entry_sector = file_num[fd].sector;
      file_num[fd].entry_number = i;
      file_num[fd].file_sector = 0;
      sdfat_select_cluster(fd, file_num[fd].full_first_cluster);
      break;
    }
  }

  int m, n;
  for(m=0;m<4;m++) {
    for(n=0;n<8;n++) {
      iprintf("%02X ", *(file_num[fd].filename + (m * 4 + n)));
    }
    iprintf("\r\n");
  }

  return 0;
}

int sdfat_read(int fd, void *buffer, int count) {
  int i=0;
  char *bt = (char *)buffer;
//  iprintf("sdfat_read\n");
//  iprintf("read %d bytes from file %d\n", count, fd);
//  iprintf("file size %d bytes, cursor at %d\n",
//          file_num[fd].size,
//          file_num[fd].cursor + file_num[fd].file_sector * 512);
  while(i < count) {
    if(((file_num[fd].cursor + file_num[fd].file_sector * 512)) >= file_num[fd].size) {
      break;   /* end of file */
    }
    *bt++ = *(char *)(file_num[fd].buffer + file_num[fd].cursor);
    file_num[fd].cursor++;
    if(file_num[fd].cursor == 512) {
      sdfat_next_sector(fd);
    }
    i++;
  }
//  iprintf("actually read %d bytes\n", i);
  return i;
}

int sdfat_lseek(int fd, int ptr, int dir) {
  unsigned int new_pos;
  unsigned int old_pos;
  int new_sec;
  int i;
  int file_cluster;

  if(!(available_files & (1 << fd))) {
    return ptr-1;    /* tried to seek on a file that's not open */
  }
  
  if(file_num[fd].dirty) {
    sdfat_flush(fd);
  }
  old_pos = file_num[fd].file_sector * 512 + file_num[fd].cursor;
  if(dir == SEEK_SET) {
    new_pos = ptr;
  } else if(dir == SEEK_CUR) {
    new_pos = file_num[fd].file_sector * 512 + file_num[fd].cursor + ptr;
  } else {
    new_pos = file_num[fd].size + ptr;
  }
  if(new_pos > file_num[fd].size) {
    return ptr-1; /* tried to seek outside a file */
  }
  // optimisation cases
  if((old_pos/512) == (new_pos/512)) {
    // case 1: seeking within a disk block
    file_num[fd].cursor = new_pos & 0x1ff;
    return new_pos;
  } else if((new_pos / (card.sectors_per_cluster * 512)) == (old_pos / (card.sectors_per_cluster * 512))) {
    // case 2: seeking within the cluster, just need to hope forward/back some sectors
    file_num[fd].file_sector = new_pos / 512;
    file_num[fd].sector = file_num[fd].sector + (new_pos/512) - (old_pos/512);
    file_num[fd].sectors_left = file_num[fd].sectors_left + (new_pos/512) - (old_pos/512);
    file_num[fd].cursor = new_pos & 0x1ff;
    if(sd_read_block(file_num[fd].buffer, file_num[fd].sector)) {
      return ptr - 1;
    }
    return new_pos;
  }
  // otherwise we need to seek the cluster chain
  file_cluster = new_pos / (card.sectors_per_cluster * 512);
  
  file_num[fd].cluster = file_num[fd].full_first_cluster;
  i = 0;
  // walk the FAT cluster chain until we get to the right one
  while(i<file_cluster) {
    file_num[fd].cluster = sdfat_next_cluster(fd);
    i++;
  }
  file_num[fd].file_sector = new_pos / 512;
  file_num[fd].cursor = new_pos & 0x1ff;
  new_sec = new_pos - file_cluster * card.sectors_per_cluster * 512;
  new_sec = new_sec / 512;
  file_num[fd].sector = file_num[fd].cluster * card.sectors_per_cluster + card.cluster0 + new_sec;
  file_num[fd].sectors_left = card.sectors_per_cluster - new_sec - 1;
  if(sd_read_block(file_num[fd].buffer, file_num[fd].sector)) {
    return ptr-1;
  }
  return new_pos;
}

int sdfat_stat(int fd, struct stat *st) {
  st->st_dev = 0;
  st->st_ino = 0;
  if(file_num[fd].attributes & FAT_ATT_SUBDIR) {
    st->st_mode = S_IFDIR;
  } else {
    st->st_mode = S_IFREG;
  }
  st->st_nlink = 1;   /* number of hard links to the file */
  st->st_uid = 0;
  st->st_gid = 0;     /* not implemented on FAT */
  st->st_rdev = 0;
  st->st_size = file_num[fd].size;
  /* should be seconds since epoch. */
  st->st_atime = sdfat_to_unix_time(file_num[fd].access_date);
  st->st_mtime = (sdfat_to_unix_date(file_num[fd].modified_date) +
                  sdfat_to_unix_time(file_num[fd].modified_time));
  st->st_ctime = (sdfat_to_unix_date(file_num[fd].create_date) +
                  sdfat_to_unix_time(file_num[fd].create_time));
  st->st_blksize = 512;
  st->st_blocks = 1;  /* number of blocks allocated for this object */
  return 0; 
}

int sdfat_get_next_dirent(int fd, struct dirent *out_de) {
  direntS *de;

  de = (direntS *)(file_num[fd].buffer + file_num[fd].cursor);

  /* first check the current entry isn't the end of the folder */
  if(de->filename[0] == 0) {
    return -1;
  }
  /* now keep looping past LFN entries until a valid one or the end of dir is found */
  while(1) {
    /* otherwise look for the next entry */
    if(file_num[fd].cursor + 32 == 512) {
      if(sdfat_next_sector(fd) == -1) {
        return -1;  /* there are no more sectors allocated to this directory */
      }
    } else {
      file_num[fd].cursor += 32;
    }
    de = (direntS *)(file_num[fd].buffer + file_num[fd].cursor);
    if(de->filename[0] == 0) {
      return -1;
    }
    if(!((de->attributes == 0x0F) || (de->attributes & FAT_ATT_VOL))) {
      /* if it's not an LFN and not a volume label it's a real file. */
      fatname_to_str(out_de->d_name, de->filename);
      if(card.fs == PART_TYPE_FAT16) {
        out_de->d_ino = de->first_cluster;
      } else {
        out_de->d_ino = de->first_cluster + (de->high_first_cluster << 16);
      }
      return 0;
    }
  }
}

char *sdfat_open_media(char *filename) {
  int fn;
  iprintf("sdfat_open_media - entry\r\n");
  fn = sdfat_open(filename, O_RDONLY);
  media_file.cluster = file_num[fn].full_first_cluster;
  media_file.file_len = file_num[fn].size;
  media_file.nearly_near_end = 0;
  media_file.near_end = 0;
  media_file.meta_block = 0;
  media_file.active_buffer = 0;
  media_file.block_count = 0;
  media_file.block = media_file.cluster * card.sectors_per_cluster + card.cluster0;
  /* ought to check cluster size here !! */
  media_file.buffer_ready[0] = 1;
  sd_read_multiblock(media_file.buffer[0], media_file.block, 4, &media_file.buffer_ready[0]);
  media_file.block += 4;
  media_file.block_count += 4;
  media_file.buffer_ready[1] = 1;
  sd_read_multiblock(media_file.buffer[1], media_file.block, 4, &media_file.buffer_ready[1]);
  /* don't need the fd we used to open the file with now */
  sdfat_close(fn);
//   iprintf("media_file.cluster = %d\r\n", media_file.cluster);
//   iprintf("sdfat_open_media - exit\r\n");
//   int i, j;
//   for(i=0;i<16;i++) {
//     for(j=0;j<32;j++) {
//       iprintf("%02X ", media_file.buffer[0][i*32 + j]);
//     }
//     for(j=0;j<32;j++) {
//       iprintf("%c", media_file.buffer[0][i*32 + j]);
//     }
//     iprintf("\r\n");
//   }
  //while(1) {;}
  return media_file.buffer[0];
}

int media_get_next_cluster() {
  unsigned int i, j;
  volatile uint32_t flag;
//  iprintf("media_get_next_cluster - entry\n");
//  iprintf("media_file.cluster = %d\n", media_file.cluster);
  i = media_file.cluster;
  i = i * card.fat_entry_len;     /* either 2 bytes for FAT16 or 4 for FAT32 */
  j = (i / 512) + card.active_fat_start; /* get the sector number we want */
  //iprintf("Reading sector %d of FAT at %d for cluster %d\n", (i / 512), card.active_fat_start + (i / 512), file_num[fd].cluster);
  if(j != media_file.meta_block) {
    flag = 1;
    sd_read_multiblock(media_file.meta_buffer, j, 1, &flag);
    while(flag) {__asm__("nop\n\t");}
//       return -1;
//     }
    media_file.meta_block = j;
  }
  i = i & 0x1FF;
  j = media_file.meta_buffer[i++];
  j += (media_file.meta_buffer[i++] << 8);
  if(card.fs == PART_TYPE_FAT32) {
    j += media_file.meta_buffer[i++] << 16;
    j += media_file.meta_buffer[i++] << 24;
  }
  if(j < 2) {
    media_file.error = FAT_ERROR_CLUSTER;
    iprintf("ERROR CLUSTER %08X\n", j);
    return -1;
  } else if(j >= card.end_cluster_marker) {
    iprintf("END OF FILE %08X\n", j);
    media_file.error = FAT_END_OF_FILE;
    return -1;
  }
  media_file.cluster = j;
//  iprintf("media_get_next_cluster - exit\n");
  return 0;
}

/**
 *  sdfat_read_media - returns a pointer to the next 2k section of the open
 *                     file
 */
char *sdfat_read_media() {
//  iprintf("sdfat_read_media - entry\n");
//  iprintf("media_file.block = %d\n", media_file.block);
  /* setup read-ahead fetch */
  if(!media_file.nearly_near_end) {
    media_file.block_count += 4;
    media_file.block += 4;
    if(media_file.block_count * 512 > media_file.file_len) {
      media_file.nearly_near_end = 1;
      media_file.file_end = media_file.file_len - ((media_file.block_count - 4) * 512);
    }
    if((media_file.block_count % card.sectors_per_cluster) == 0) {
//    iprintf("media_file.block_count = %d, card.sectors_per_cluster = %d\n", media_file.block_count, card.sectors_per_cluster);
      /* need to get the next cluster */
      media_get_next_cluster();
      media_file.block = card.cluster0 + media_file.cluster * card.sectors_per_cluster;
    }
    media_file.buffer_ready[media_file.active_buffer] = 1;
    sd_read_multiblock(media_file.buffer[media_file.active_buffer], media_file.block, 4, &media_file.buffer_ready[media_file.active_buffer]);
  } else {
    media_file.near_end = 1;
  }
  media_file.active_buffer ^= 1; // (media_file.active_buffer + 1) % 2;
//  iprintf("sdfat_read_media - exit\n\n");
  /* check that the data we're returning is ready */
  while(media_file.buffer_ready[media_file.active_buffer]) {__asm__("nop\n\r");}
  return media_file.buffer[media_file.active_buffer];
}

