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

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "dirent.h"
#include <errno.h>
#include "block.h"
#include "fat.h"
#include "mbr.h"

/**
 * global variable structures.
 * These take the place of a real operating system.
 **/

struct fat_info fatfs;
FileS file_num[MAX_OPEN_FILES];
// uint32_t available_files;

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

uint16_t fat_from_unix_time(time_t seconds) {
  struct tm *time_str;
  uint16_t fattime;
  time_str = gmtime(&seconds);
  
  fattime = 0;
  
  fattime += time_str->tm_hour << 11;
  fattime += time_str->tm_min << 5;
  fattime += time_str->tm_sec >> 1;
  return fattime;
}

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

uint16_t fat_from_unix_date(time_t seconds) {
  struct tm *time_str;
  uint16_t fatdate;
  
  time_str = gmtime(&seconds);
  
  fatdate = 0;
  
  fatdate += (time_str->tm_year - 80) << 9;
  fatdate += (time_str->tm_mon + 1) << 5;
  fatdate += time_str->tm_mday;
  
  return fatdate;
}

/*
 * fat_update_atime - Updates the access date on the selected file
 * 
 * since FAT only stores an access date it's highly likely this won't change from the last
 * time it was accessed, a test is made, if this is the case, the fs_dirty flag is not set
 * so no flush is required on the meta info for this file.
 */
int fat_update_atime(int fd) {
  uint16_t new_date, old_date;
  new_date = fat_from_unix_date(time(NULL));
  old_date = fat_from_unix_date(file_num[fd].accessed);
  
  if(old_date != new_date) {
    file_num[fd].accessed = time(NULL);
    file_num[fd].flags |= FAT_FLAG_FS_DIRTY;
  }
  
  return 0;
}

/*
 * fat_update_mtime - Updates the modified time and date on the selected file
 * 
 * Since this is tracked to the nearest 2 seconds it is assumed there will always be an update
 * so to reduce overheads, the date is just set and the fs_dirty flag set.
 */
int fat_update_mtime(int fd) {
  file_num[fd].modified = time(NULL);
  file_num[fd].flags |= FAT_FLAG_FS_DIRTY;
  return 0;
}

/* sdfat_get_next_file - returns the next free file descriptor or -1 if none */
int8_t fat_get_next_file() {
  int j;

  for(j=0;j<MAX_OPEN_FILES;j++) {
    if((file_num[j].flags & FAT_FLAG_OPEN) == 0) {
      file_num[j].flags = FAT_FLAG_OPEN;
      return j;
    }
  }
  return -1;
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

//   iprintf("path input = %s\n", path);

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
//       iprintf("Exit 1\n");
      return -1;
    } else {
      *(dosname + i) = c;
      c = doschar(*(path + (*path_pointer)++));
    }
  }
//   iprintf("main exit char = %c (%x)\n", c, c);
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
//       iprintf("Exit 2\n");
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
//   iprintf("dosname = %s, last char = %c (%x)\n", dosname, *(path + (*path_pointer)), *(path + (*path_pointer)));
  if((c == '/') || (c == 0)) {
    return 0; /* extension ends the filename. */
  } else {
//     iprintf("Exit 3\n");
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

int fat_get_free_cluster() {
  blockno_t i;
  int j;
  uint32_t e;
  for(i=fatfs.active_fat_start;i<fatfs.active_fat_start + fatfs.sectors_per_fat;i++) {
    if(block_read(i, file_num[fd].buffer)) {
      return 0xFFFFFFFF;
    }
    for(j=0;j<(512/fatfs.fat_entry_len);j++) {
      e = file_num[fd].buffer[j*fatfs.fat_entry_len];
      e += file_num[fd].buffer[j*fatfs.fat_entry_len+1] << 8;
      if(fatfs.type == PART_TYPE_FAT32) {
        e += file_num[fd].buffer[j*fatfs.fat_entry_len+2];
        e += file_num[fd].buffer[j*fatfs.fat_entry_len+3];
      }
      if(e == 0) {
        /* this is a free cluster */
        /* first, mark it as the end of the chain */
        if(fatfs.type == PART_TYPE_FAT16) {
          file_num[fd].buffer[j*fatfs.fat_entry_len] = 0xFF;
          file_num[fd].buffer[j*fatfs.fat_entry_len+1] = 0x8F;
        } else {
          file_num[fd].buffer[j*fatfs.fat_entry_len] = 0xFF;
          file_num[fd].buffer[j*fatfs.fat_entry_len+1] = 0xFF;
          file_num[fd].buffer[j*fatfs.fat_entry_len+2] = 0xFF;
          file_num[fd].buffer[j*fatfs.fat_entry_len+3] = 0x8F;
        }
        if(block_write(i, file_num[fd].buffer)) {
          return 0xFFFFFFFF;
        }
        return e;
      }
    }
  }
  return 0;     /* no clusters found, should raise ENOSPC */
}

/* write a sector back to disc */
int fat_flush(int fd) {
  /* only write to disk if we need to */
  if(file_num[fd].flags & FAT_FLAG_DIRTY) {
    if(file_num[fd].sector == 0) {
      /* this is a new file that's never been saved before, it needs a new cluster
       * assigned to it, the data stored, then the meta info flushed */
      file_num[fd].cluster = ??;
      file_num[fd].sector = ??;
      
    if(block_write(file_num[fd].sector, file_num[fd].buffer)) {
      /* write failed, don't clear the dirty flag */
      return -1;
    }
    /* just clear this flag so this isn't called every time from now on */
    file_num[fd].flags &= ~FAT_FLAG_DIRTY;
  }
  return 0;
}

/*
 * fat_free_clusters - starts at given cluster and marks all as free until an
 *                     end of chain marker is found
 */
int fat_free_clusters(int fd, uint32_t cluster) {
  int estart;
  uint32_t j;
  blockno_t current_block = MAX_BLOCK;
  
  while(1) {
    if(fatfs.active_fat_start + ((cluster * fatfs.fat_entry_len) / 512) != current_block) {
      if(current_block != MAX_BLOCK) {
        block_write(current_block, file_num[fd].buffer);
      }
      if(block_read(fatfs.active_fat_start + ((cluster * fatfs.fat_entry_len) / 512), file_num[fd].buffer)) {
        return -1;
      }
      current_block = fatfs.active_fat_start + ((cluster * fatfs.fat_entry_len)/512);
    }
    estart = (cluster * fatfs.fat_entry_len) & 0x1ff;
    j = file_num[fd].buffer[estart];
    file_num[fd].buffer[estart] = 0;
    j += file_num[fd].buffer[estart + 1] << 8;
    file_num[fd].buffer[estart+1] = 0;
    if(fatfs.type == PART_TYPE_FAT32) {
      j += file_num[fd].buffer[estart + 2];
      file_num[fd].buffer[estart+2] = 0;
      j += file_num[fd].buffer[estart + 3];
      file_num[fd].buffer[estart+3] = 0;
    }
    cluster = j;
    if(cluster >= fatfs.end_cluster_marker) {
      break;
    }
  }
  block_write(current_block, file_num[fd].buffer);
  
  return 0;
}

// int sdfat_select_sector(int fd, blockno_t sector) {
//   if(fat_flush(fd)) {
//     return -1;
//   }
//   file_num[fd].sector = sector;
//   file_num[fd].sectors_left = (sector - fatfs.cluster0) % fatfs.sectors_per_cluster;
//   file_num[fd].cluster = (sector - fatfs.cluster0) / fatfs.sectors_per_cluster;
//   file_num[fd].cursor = 0;
//   return block_read(file_num[fd].sector, file_num[fd].buffer);
// }

/* get the first sector of a given cluster */
int fat_select_cluster(int fd, uint32_t cluster) {

  file_num[fd].sector = cluster * fatfs.sectors_per_cluster + fatfs.cluster0;
  file_num[fd].sectors_left = fatfs.sectors_per_cluster - 1;
  file_num[fd].cluster = cluster;
  file_num[fd].cursor = 0;

  return block_read(file_num[fd].sector, file_num[fd].buffer);
}

/* get the next cluster in the current file */
int sdfat_next_cluster(int fd, int *rerrno) {
  uint32_t i;
  uint32_t j;
  uint32_t k;

  if(fat_flush(fd)) {
    return -1;
  }
  i = file_num[fd].cluster;
  i = i * fatfs.fat_entry_len;     /* either 2 bytes for FAT16 or 4 for FAT32 */
  j = (i / 512) + fatfs.active_fat_start; /* get the sector number we want */
  if(block_read(j, file_num[fd].buffer)) {
    return -1;
  }
  i = i & 0x1FF;
  j = file_num[fd].buffer[i++];
  j += (file_num[fd].buffer[i++] << 8);
  if(fatfs.type == PART_TYPE_FAT32) {
    j += file_num[fd].buffer[i++] << 16;
    j += file_num[fd].buffer[i++] << 24;
  }
  if(j < 2) {
    file_num[fd].error = FAT_ERROR_CLUSTER;
    return -1;
  } else if(j >= fatfs.end_cluster_marker) {
    if(file_num[fd].flags & FAT_FLAG_WRITE) {
      /* opened for writing, we can extend the file */
      /* find the first available cluster */
      k = fat_get_free_cluster(fd);
      if(k == 0) {
        (*rerrno) = ENOSPC;
        return -1;
      }
      if(k == 0xFFFFFFFF) {
        (*rerrno) = EIO;
        return -1;
      }
      i = file_num[fd].cluster;
      i = i * fatfs.fat_entry_len;
      j = (i/512) + fatfs.active_fat_start;
      if(block_read(j, file_num[fd].buffer)) {
        (*rerrno) = EIO;
        return -1;
      }
      /* update the pointer to the new end of chain */
      memcpy(&file_num[fd].buffer[i & 0x1FF], &k, 4);
      if(block_write(j, file_num[fd].buffer)) {
        (*rerrno) = EIO;
        return -1;
      }
      j = k;
    } else {
      /* end of the file cluster chain reached */
      file_num[fd].error = FAT_END_OF_FILE;
      (*rerrno) = 0;
      return -1;
    }
  }
  return j;
}

/* get the next sector in the current file. */
int fat_next_sector(int fd) {
  int c;
  int rerrno;
  /* if the current sector was written write to disc */
  if(fat_flush(fd)) {
    return -1;
  }
  /* see if we need another cluster */
  if(file_num[fd].sectors_left > 0) {
    file_num[fd].sectors_left--;
    file_num[fd].file_sector++;
    file_num[fd].cursor = 0;
    return block_read(++file_num[fd].sector, file_num[fd].buffer);
  } else {
    c = sdfat_next_cluster(fd, &rerrno);
    if(c > -1) {
      file_num[fd].file_sector++;
      return fat_select_cluster(fd, sdfat_next_cluster(fd, &rerrno));
    } else {
      return -1;
    }
  }
}

/* Function to save file meta-info, (size modified date etc.) */
int fat_flush_fileinfo(int fd) {
  direntS de;
  direntS *de2;
  int i;
  uint32_t temp_sectors_left;
  uint32_t temp_file_sector;
  uint32_t temp_cluster;
  uint32_t temp_sector;
  uint32_t temp_cursor;
  
  if(file_num[fd].full_first_cluster == fatfs.root_cluster) {
    // do nothing to try and update meta info on the root directory
    return 0;
  }
  memcpy(de.filename, file_num[fd].filename, 8);
  memcpy(de.extension, file_num[fd].extension, 3);
  de.attributes = file_num[fd].attributes;
  /* fine resolution = 10ms, only using unix time stamp so save
   * the unit second, create_time only saves in 2s resolution */
  de.create_time_fine = (file_num[fd].created & 1) * 100;
  de.create_time = fat_from_unix_time(file_num[fd].created);
  de.create_date = fat_from_unix_date(file_num[fd].created);
  de.access_date = fat_from_unix_date(file_num[fd].accessed);
  de.high_first_cluster = file_num[fd].full_first_cluster >> 16;
  de.modified_time = fat_from_unix_time(file_num[fd].modified);
  de.modified_date = fat_from_unix_date(file_num[fd].modified);
  de.first_cluster = file_num[fd].full_first_cluster & 0xffff;
  de.size = file_num[fd].size;
  
  /* make sure the buffer has no changes in it */
  if(fat_flush(fd)) {
    return -1;
  }
  if(file_num[fd].entry_sector == 0) {
    /* this is a new file that's never been written to disc */
    // save the tracking info for this file, we'll need to seek through the parent with
    // this file descriptor
    temp_sectors_left = file_num[fd].sectors_left;
    temp_file_sector = file_num[fd].file_sector;
    temp_cursor = file_num[fd].cursor;
    temp_sector = file_num[fd].sector;
    temp_cluster = file_num[fd].cluster;
    
    fat_select_cluster(fd, file_num[fd].parent_cluster);
    
    // find the first empty file location in the directory
    while(1) {
      // 16 entries per disc block
      for(i=0;i<16;i++) {
        de2 = (direntS *)(file_num[fd].buffer + file_num[fd].cursor);
        if(de2->filename[0] == 0) {
          // this is an empty entry
          break;
        }
      }
      if(i < 16) {
        // we found an empty in this block
        break;
      }
      fat_next_sector(fd);
    }
    
    // save the entry_sector and entry_number
    file_num[fd].entry_sector = file_num[fd].sector;
    file_num[fd].entry_number = i;
    
    // restore the file tracking info
    file_num[fd].sectors_left = temp_sectors_left;
    file_num[fd].file_sector = temp_file_sector;
    file_num[fd].cursor = temp_cursor;
    file_num[fd].sector = temp_sector;
    file_num[fd].cluster = temp_cluster;
  } else {
    /* read the directory entry for this file */
    if(block_read(file_num[fd].entry_sector, file_num[fd].buffer)) {
      return -1;
    }
  }
  /* copy the new entry over the old */
  memcpy(&file_num[fd].buffer[file_num[fd].entry_number * 32], &de, 32);
  /* write the modified directory entry back to disc */
  if(block_write(file_num[fd].entry_sector, file_num[fd].buffer)) {
    return -1;
  }
  /* fetch the sector that was expected back into the buffer */
  if(block_read(file_num[fd].sector, file_num[fd].buffer)) {
    return -1;
  }
  /* mark the filesystem as consistent now */
  file_num[fd].flags &= ~FAT_FLAG_FS_DIRTY;
  return 0;
}

int fat_lookup_path(int fd, const char *path, int *rerrno) {
  char dosname[12];
  char isdir;
  int i;
  int path_pointer = 0;
  direntS *de;
  
  /* Make sure the file system has all changes flushed before searching it */
  for(i=0;i<MAX_OPEN_FILES;i++) {
    if(file_num[i].flags & FAT_FLAG_FS_DIRTY) {
      fat_flush_fileinfo(i);
    }
  }

  if(path[0] != '/') {
    (*rerrno) = ENOENT;
    return -1;                                /* bad path, we have no cwd */
  }

  /* select root directory */
  fat_select_cluster(fd, fatfs.root_cluster);

  path_pointer++;

  if(*(path + path_pointer) == 0) {
    /* user selected the root directory to open. */
    file_num[fd].full_first_cluster = fatfs.root_cluster;
    file_num[fd].entry_sector = 0;
    file_num[fd].entry_number = 0;
    file_num[fd].file_sector = 0;
    file_num[fd].attributes = FAT_ATT_SUBDIR;
    file_num[fd].size = 4096;
    file_num[fd].accessed = 0;
    file_num[fd].modified = 0;
    file_num[fd].created = 0;
    fat_select_cluster(fd, file_num[fd].full_first_cluster);
    return 0;
  }

  while(1) {
    if(make_dos_name(dosname, path, &path_pointer)) {
      (*rerrno) = ENOENT;
      return -1;  /* invalid path name */
    }
//     printf("%s\r\n", dosname);
    while(1) {
//       printf("looping [s:%d/%d c:%d]\r\n", file_num[fd].sectors_left, fatfs.sectors_per_cluster, file_num[fd].cluster);
      for(i=0;i<16;i++) {
        if(strncmp(dosname, (char *)(file_num[fd].buffer + (i * 32)), 11) == 0) {
          break;
        }
//         file_num[fd].buffer[i * 32 + 11] = 0;
//         printf("%s %d\r\n", (char *)(file_num[fd].buffer + (i * 32)), i);
      }
      if(i == 16) {
        if(fat_next_sector(fd) != 0) {
          memcpy(file_num[fd].filename, dosname, 8);
          memcpy(file_num[fd].extension, dosname+8, 3);
          (*rerrno) = ENOENT;
          return -1;
        }
      } else {
        break;
      }
    }
//     printf("got here %d\r\n", i);
    de = (direntS *)(file_num[fd].buffer + (i * 32));
//     iprintf("%s\r\n", de->filename);
    isdir = de->attributes & 0x10;
    /* if dir, and there are more path elements, select */
    if(isdir && (doschar(path[path_pointer]) == '/') && (doschar(path[path_pointer + 1]) != 0)) {
      path_pointer++;
      if(fatfs.type == PART_TYPE_FAT16) {
        if(de->first_cluster == 0) {
          file_num[fd].parent_cluster = fatfs.root_cluster;
          file_num[fd].parent_attributes = FAT_ATT_SUBDIR;
          fat_select_cluster(fd, fatfs.root_cluster);
        } else {
          file_num[fd].parent_cluster = de->first_cluster;
          file_num[fd].parent_attributes = de->attributes;
          fat_select_cluster(fd, de->first_cluster);
        }
      } else {
        if(de->first_cluster + (de->high_first_cluster << 16) == 0) {
          file_num[fd].parent_cluster = fatfs.root_cluster;
          file_num[fd].parent_attributes = FAT_ATT_SUBDIR;
          fat_select_cluster(fd, fatfs.root_cluster);
        } else {
          file_num[fd].parent_cluster = de->first_cluster + (de->high_first_cluster << 16);
          file_num[fd].parent_attributes = de->attributes;
          fat_select_cluster(fd, de->first_cluster + (de->high_first_cluster << 16));
        }
      }
    } else if((doschar(path[path_pointer]) == '/') && (doschar(path[path_pointer+1]) != 0)) {
      /* path end not reached but this is not a directory */
      (*rerrno) = ENOTDIR;
      return -1;
    } else {
      /* otherwise, setup the fd */
      file_num[fd].error = 0;
      file_num[fd].flags = FAT_FLAG_OPEN;
      memcpy(file_num[fd].filename, de->filename, 8);
      memcpy(file_num[fd].extension, de->extension, 3);
      file_num[fd].attributes = de->attributes;
      file_num[fd].size = de->attributes;
      if(fatfs.type == PART_TYPE_FAT16) {
        file_num[fd].full_first_cluster = de->first_cluster;
      } else {
        file_num[fd].full_first_cluster = de->first_cluster + (de->high_first_cluster << 16);
      }

      /* this following special case occurs when a subdirectory's .. entry is opened. */
      if(file_num[fd].full_first_cluster == 0) {
        file_num[fd].full_first_cluster = fatfs.root_cluster;
      }

      file_num[fd].entry_sector = file_num[fd].sector;
      file_num[fd].entry_number = i;
      file_num[fd].file_sector = 0;
      
      file_num[fd].created = fat_to_unix_date(de->create_date) + fat_to_unix_time(de->create_time) + de->create_time_fine;
      file_num[fd].modified = fat_to_unix_date(de->modified_date) + fat_to_unix_time(de->modified_date);
      file_num[fd].accessed = fat_to_unix_date(de->access_date);
      fat_select_cluster(fd, file_num[fd].full_first_cluster);
      break;
    }
  }

  return 0;
}

/**
 * callable file access routines
 */

int fat_mount(blockno_t part_start, uint8_t filesystem) {
  int i;
  boot_sector_fat16 *boot16;
  boot_sector_fat32 *boot32;
  
  fatfs.read_only = block_get_device_read_only();
  block_read(part_start, fatfs.sysbuf);
  if(filesystem == PART_TYPE_FAT16) {
    fatfs.fat_entry_len = 2;
    fatfs.end_cluster_marker = 0xFFF0;
    boot16 = (boot_sector_fat16 *)fatfs.sysbuf;
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
    boot32 = (boot_sector_fat32 *)fatfs.sysbuf;
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

int fat_open(const char *name, int flags, int mode, int *rerrno) {
  int i;
  int8_t fd;
  (*rerrno) = 0;
  
//   printf("fat_open(%s, %x)\n", name, mode);
  fd = fat_get_next_file();
  if(fd < 0) {
    (*rerrno) = ENFILE;
    return -1;   /* too many open files */
  }
  if((flags & O_RDWR)) {
    file_num[fd].flags |= (FAT_FLAG_READ | FAT_FLAG_WRITE);
  } else {
    if((flags & O_WRONLY) == 0) {
      file_num[fd].flags |= FAT_FLAG_READ;
    } else {
      file_num[fd].flags |= FAT_FLAG_WRITE;
    }
  }
  
  if(flags & O_APPEND) {
    file_num[fd].flags |= FAT_FLAG_APPEND;
  }
//   printf("Lookup path\n");
  i = fat_lookup_path(fd, name, rerrno);
  if((i == -1) && ((*rerrno) == ENOENT)) {
    /* file doesn't exist */
    if((flags & (O_CREAT)) == 0) {
      /* tried to open a non-existent file with no create */
      file_num[fd].flags = 0;
      (*rerrno) = ENOENT;
      return -1;
    } else {
      /* opening a new file for writing */
      /* only create files in directories that aren't read only */
      if(fatfs.read_only) {
        file_num[fd].flags = 0;
        (*rerrno) = EROFS;
        return -1;
      }
      if(file_num[fd].parent_attributes & FAT_ATT_RO) {
        file_num[fd].flags = 0;
        (*rerrno) = EACCES;
        return -1;
      }
      /* create an empty file structure ready for use */
      file_num[fd].sector = 0;
      file_num[fd].cluster = 0;
      file_num[fd].sectors_left = 0;
      file_num[fd].cursor = 0;
      file_num[fd].error = 0;
      if(mode & S_IWUSR) {
        file_num[fd].attributes = 0;
      } else {
        file_num[fd].attributes = FAT_ATT_RO;
      }
      file_num[fd].size = 0;
      file_num[fd].full_first_cluster = 0;
      file_num[fd].entry_sector = 0;
      file_num[fd].entry_number = 0;
      file_num[fd].file_sector = 0;
      file_num[fd].created = time(NULL);
      file_num[fd].modified = 0;
      file_num[fd].accessed = 0;
      
      memset(file_num[fd].buffer, 0, 512);
      
      file_num[fd].flags |= FAT_FLAG_FS_DIRTY;
      return fd;
    }
  } else if(i == 0) {
    /* file does exist */
    if(flags & (O_CREAT | O_EXCL)) {
      /* tried to force creation of an existing file */
      file_num[fd].flags = 0;
      (*rerrno) = EEXIST;
      return -1;
    } else {
      if((flags & (O_WRONLY | O_RDWR)) == 0) {
        /* read existing file */
        file_num[fd].file_sector = 0;
        return fd;
      } else {
        /* file opened for write access, check permissions */
        if(fatfs.read_only) {
          /* requested write on read only filesystem */
          file_num[fd].flags = 0;
          (*rerrno) = EROFS;
          return -1;
        }
        if(file_num[fd].attributes & FAT_ATT_RO) {
          /* The file is read-only refuse permission */
          file_num[fd].flags = 0;
          (*rerrno) = EACCES;
          return -1;
        }
        if(file_num[fd].attributes & FAT_ATT_SUBDIR) {
          /* Tried to open a directory for writing */
          file_num[fd].flags = 0;
          (*rerrno) = EISDIR;
          return -1;
        }
        if(flags & O_TRUNC) {
          /* Need to truncate the file to zero length */
          fat_free_clusters(fd, file_num[fd].full_first_cluster);
          file_num[fd].size = 0;
          file_num[fd].full_first_cluster = 0;
          file_num[fd].sector = 0;
          file_num[fd].cluster = 0;
          file_num[fd].sectors_left = 0;
          file_num[fd].file_sector = 0;
          file_num[fd].created = time(NULL);
          file_num[fd].modified = time(NULL);
          file_num[fd].flags |= FAT_FLAG_FS_DIRTY;
        }
        file_num[fd].file_sector = 0;
        return fd;
      }
    }
  } else {
    return -1;
  }
}

int fat_close(int fd, int *rerrno) {
  (*rerrno) = 0;
  if(fd >= MAX_OPEN_FILES) {
    (*rerrno) = EBADF;
    return -1;
  }
  if(!(file_num[fd].flags & FAT_FLAG_OPEN)) {
    (*rerrno) = EBADF;
    return -1;
  }
  if(file_num[fd].flags & FAT_FLAG_DIRTY) {
    if(fat_flush(fd)) {
      (*rerrno) = EIO;
      return -1;
    }
  }
  if(file_num[fd].flags & FAT_FLAG_FS_DIRTY) {
    if(fat_flush_fileinfo(fd)) {
      (*rerrno) = EIO;
      return -1;
    }
  }
  file_num[fd].flags = 0;
  return 0;
}

int fat_read(int fd, void *buffer, size_t count, int *rerrno) {
  int i=0;
  uint8_t *bt = (uint8_t *)buffer;
  (*rerrno) = 0;
  if(fd >= MAX_OPEN_FILES) {
    (*rerrno) = EBADF;
    return -1;
  }
  if((~file_num[fd].flags) & (FAT_FLAG_OPEN | FAT_FLAG_READ)) {
    (*rerrno) = EBADF;
    return -1;
  }
  while(i < count) {
    if(((file_num[fd].cursor + file_num[fd].file_sector * 512)) >= file_num[fd].size) {
      break;   /* end of file */
    }
    *bt++ = *(uint8_t *)(file_num[fd].buffer + file_num[fd].cursor);
    file_num[fd].cursor++;
    if(file_num[fd].cursor == 512) {
      fat_next_sector(fd);
    }
    i++;
  }
  if(i > 0) {
    fat_update_atime(fd);
  }
  return i;
}

int fat_write(int fd, const void *buffer, size_t count, int *rerrno) {
  int i=0;
  uint8_t *bt = (uint8_t *)buffer;
  (*rerrno) = 0;
  if(fd >= MAX_OPEN_FILES) {
    (*rerrno) = EBADF;
    return -1;
  }
  if((~file_num[fd].flags) & (FAT_FLAG_OPEN | FAT_FLAG_WRITE)) {
    (*rerrno) = EBADF;
    return -1;
  }
  if(file_num[fd].flags & FAT_FLAG_APPEND) {
    fat_lseek(fd, 0, SEEK_END, rerrno);
  }
  while(i < count) {
    if(((file_num[fd].cursor + file_num[fd].file_sector * 512)) == file_num[fd].size) {
      file_num[fd].size++;
      file_num[fd].flags |= FAT_FLAG_DIRTY;
    }
    file_num[fd].buffer[file_num[fd].cursor] = *bt++;
    file_num[fd].cursor++;
    if(file_num[fd].cursor == 512) {
      fat_next_sector(fd);
    }
    i++;
  }
  if(i > 0) {
    fat_update_mtime(fd);
  }
  return i;
}

int fat_fstat(int fd, struct stat *st, int *rerrno) {
  (*rerrno) = 0;
  if(fd >= MAX_OPEN_FILES) {
    (*rerrno) = EBADF;
    return -1;
  }
  if(!(file_num[fd].flags & FAT_FLAG_OPEN)) {
    (*rerrno) = EBADF;
    return -1;
  }
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
  st->st_atime = file_num[fd].accessed;
  st->st_mtime = file_num[fd].modified;
  st->st_ctime = file_num[fd].created;
  st->st_blksize = 512;
  st->st_blocks = 1;  /* number of blocks allocated for this object */
  return 0; 
}

int fat_lseek(int fd, int ptr, int dir, int *rerrno) {
  unsigned int new_pos;
  unsigned int old_pos;
  int new_sec;
  int i;
  int file_cluster;
  (*rerrno) = 0;

  if(fd >= MAX_OPEN_FILES) {
    (*rerrno) = EBADF;
    return ptr-1;
  }
  if(!(file_num[fd].flags & FAT_FLAG_OPEN)) {
    (*rerrno) = EBADF;
    return ptr-1;    /* tried to seek on a file that's not open */
  }
  
  fat_flush(fd);
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
  } else if((new_pos / (fatfs.sectors_per_cluster * 512)) == (old_pos / (fatfs.sectors_per_cluster * 512))) {
    // case 2: seeking within the cluster, just need to hop forward/back some sectors
    file_num[fd].file_sector = new_pos / 512;
    file_num[fd].sector = file_num[fd].sector + (new_pos/512) - (old_pos/512);
    file_num[fd].sectors_left = file_num[fd].sectors_left + (new_pos/512) - (old_pos/512);
    file_num[fd].cursor = new_pos & 0x1ff;
    if(block_read(file_num[fd].sector, file_num[fd].buffer)) {
      return ptr - 1;
    }
    return new_pos;
  }
  // otherwise we need to seek the cluster chain
  file_cluster = new_pos / (fatfs.sectors_per_cluster * 512);
  
  file_num[fd].cluster = file_num[fd].full_first_cluster;
  i = 0;
  // walk the FAT cluster chain until we get to the right one
  while(i<file_cluster) {
    file_num[fd].cluster = sdfat_next_cluster(fd, rerrno);
    i++;
  }
  file_num[fd].file_sector = new_pos / 512;
  file_num[fd].cursor = new_pos & 0x1ff;
  new_sec = new_pos - file_cluster * fatfs.sectors_per_cluster * 512;
  new_sec = new_sec / 512;
  file_num[fd].sector = file_num[fd].cluster * fatfs.sectors_per_cluster + fatfs.cluster0 + new_sec;
  file_num[fd].sectors_left = fatfs.sectors_per_cluster - new_sec - 1;
  if(block_read(file_num[fd].sector, file_num[fd].buffer)) {
    return ptr-1;
  }
  return new_pos;
}

int fat_get_next_dirent(int fd, struct dirent *out_de) {
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
      if(fat_next_sector(fd) == -1) {
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
      if(fatfs.type == PART_TYPE_FAT16) {
        out_de->d_ino = de->first_cluster;
      } else {
        out_de->d_ino = de->first_cluster + (de->high_first_cluster << 16);
      }
      return 0;
    }
  }
}
