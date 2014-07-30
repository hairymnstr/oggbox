#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include "ogg_meta.h"

uint32_t tag_split(char *buf) {
  int i;
  for(i=0;i<256;i++) {
    if(buf[i] == '=') {
      return i;
    }
    if((buf[i] >= 'a') && (buf[i] <= 'z')) {
      buf[i] -= 0x20;
    }
  }
  return 256;
}

int find_meta_start(FILE *fr) {
  char buf[4];
  char page_segments;
  int page_len;
  int i;

  fread(buf, 1, 4, fr);
  
  if(strncmp(buf, "OggS", 4) != 0) {
    return -1;
  }

  fseek(fr, 26, SEEK_SET);

  fread(&page_segments, 1, 1, fr);

  page_len = 0;

  for(i=0;i<page_segments;i++) {
    fread(buf, 1, 1, fr);
    page_len += buf[0];
  }
  
  fseek(fr, 27 + page_segments + page_len, SEEK_SET);

  fread(buf, 1, 4, fr);
  
  if(strncmp(buf, "OggS", 4) != 0) {
    return -2;
  }

  return 27 + page_segments + page_len;
}

int read_standard_tags(FILE *fr, struct meta *out) {
  uint32_t len;
  int tagcount;
  char buf[256];
  uint32_t mark;
  int i;
  uint8_t page_segments;
  int meta_start;

  // terminate all strings at zero length to start with
  out->title[0] = 0;
  out->album[0] = 0;
  out->artist[0] = 0;

  meta_start = find_meta_start(fr);

  fseek(fr, meta_start, SEEK_SET);
  fread(buf, 1, 4, fr);
  if(strncmp(buf, "OggS", 4) != 0) {
    iprintf("oops.\n");
    return -1;
  }

  fseek(fr, meta_start + 26, SEEK_SET);
  fread(&page_segments, 1, 1, fr);

  fseek(fr, meta_start + 27 + page_segments, SEEK_SET);
  fread(buf, 1, 1, fr);
  if(buf[0] != 3) {
    iprintf("not a comment packet :O\n");
    return -1;
  }
  fseek(fr, 6, SEEK_CUR);

  fread(&len, 4, 1, fr);
  fseek(fr, len, SEEK_CUR);
  fread(&tagcount, 4, 1, fr);
  for(i=0;i<tagcount;i++) {
    fread(&len, 4, 1, fr);
    if(len < 256) {
      fread(buf, 1, len, fr);
      buf[len] = 0;
    } else {
      fread(buf, 1, 255, fr);
      buf[255] = 0;
      fseek(fr, len - 255, SEEK_CUR);
    }
    mark = tag_split(buf);

    if((strncmp(buf,"ARTIST", 6) == 0) && (mark == 6)) {
      strncpy(out->artist, &buf[mark+1], META_STR_LEN);
    } else if((strncmp(buf, "TITLE", 5) == 0) && (mark == 5)) {
      strncpy(out->title, &buf[mark+1], META_STR_LEN);
    } else if((strncmp(buf, "ALBUM", 5) == 0) && (mark == 5)) {
      strncpy(out->album, &buf[mark+1], META_STR_LEN);
    } else if((strncmp(buf, "TRACKNUMBER", 11) == 0) && (mark == 11)) {
      out->track = 0;
      siscanf(&buf[mark+1], "%u", (unsigned int *)&out->track);
    } else if((strncmp(buf, "DATE", 4) == 0) && (mark == 4)) {
      out->date = 0;
      siscanf(&buf[mark+1], "%d", &out->date);
    }
  }
  return 0;
}

uint32_t ogg_track_length_millis(FILE *fp) {
  uint8_t segments;
  int32_t audio_sample_rate;
  int64_t granule;
  float samples_per_milli;
  uint32_t millis;
  char buf[4];
  int fd;
  fd = fileno(fp);
  if(fd==-1) {
    return -1;
  }
  
//   iprintf("seek 26 SET\r\n");
  lseek(fd, 26, SEEK_SET);
//   iprintf("tell=%d\r\n", ftell(fr));
  
  read(fd, &segments, 1);
//   iprintf("seek %d CUR\r\n", segments + 7 + 5);
  lseek(fd, segments + 7 + 5, SEEK_CUR);
  
  read(fd, &audio_sample_rate, 4);
  
//   iprintf("seek -4 END\r\n");
  lseek(fd, -4, SEEK_END);
  read(fd, buf, 4);
  lseek(fd, -5, SEEK_END);
  while(strncmp("OggS", buf, 4) != 0) {
    buf[3] = buf[2];
    buf[2] = buf[1];
    buf[1] = buf[0];
    read(fd, &buf[0], 1);
    
    if(lseek(fd, 0, SEEK_CUR) > 1) {
      lseek(fd, -2, SEEK_CUR);
    } else {
      return -2;
    }
  }
  
  lseek(fd, 6 + 1, SEEK_CUR); /* we want the 6th byte on, looking 1 byte before now */
  
  read(fd, &granule, 8);
  
  samples_per_milli = audio_sample_rate / 1000.0;
  
  millis = granule / samples_per_milli;

  return millis;
}
