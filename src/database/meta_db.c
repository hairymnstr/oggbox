#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "dirent.h"
#include "ogg_meta.h"
#include "meta_db.h"

#define META_PATH_LEN 256
#define MAX_NESTED_DIRS 6

struct track_entry {
  uint32_t album;
  uint32_t artist;
  uint32_t track_no;
  int32_t date;
  uint32_t disc_no;
  uint32_t length;
  char title[64];
  char path[META_PATH_LEN];
};

int path_join(char *s1, char *s2, int length, char *s3) {
  snprintf(s3, length, "%s%s", s1, s2);
  return 0;
}

int store_tracks(char *root_path) {
  struct meta meta_info;
  struct track_entry track;
  struct stat stat_buf;
  FILE *fr;
  DIR *dr;
  struct dirent *de;
  char filename[256];
  int i;
  dr = opendir(root_path);

  while((de = readdir(dr))) {
    if(de->d_name[0] != '.') {
      path_join(root_path, de->d_name, sizeof(filename), filename);
      stat(filename, &stat_buf);
      if(S_ISDIR(stat_buf.st_mode)) {
        sprintf(filename, "%s%s/", root_path, de->d_name);
        store_tracks(filename);
      }
      if(strlen(de->d_name) > 3) {
        if(strncmp(".ogg", &de->d_name[strlen(de->d_name)-4], 4) ==  0) {
          path_join(root_path, de->d_name, sizeof(filename), filename);
          fr = fopen(filename, "rb");

          read_standard_tags(fr, &meta_info);

          strncpy(track.title, meta_info.title, 64);
          track.album = 0;
          track.artist = 0;
          track.track_no = meta_info.track;
          track.date = meta_info.date;
          track.disc_no = meta_info.disc;
          strncpy(track.path, filename, META_PATH_LEN);
          fseek(fr, 0, SEEK_SET);
          track.length = ogg_track_length_millis(fr);
          printf("name %s (%d) %s\r\n", track.path, strlen(track.path), track.title);
          for(i=0;i<strlen(track.title);i++) {
            printf("%02x ", *(unsigned char *)&track.title[i]);
          }
          printf("\r\n");
          printf("album %d\r\n", track.album);
          printf("artist %d\r\n", track.artist);
          printf("track_no %d\r\n", track.track_no);
          printf("length %d\r\n", track.length);
          printf("date %d\r\n", track.date);
          printf("disc_no %d\r\n", track.disc_no);
          //fwrite(&track, sizeof(track), 1, fw);
          fclose(fr);
        }
      }
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf("select directory\r\n");
    return -1;
  }
  store_tracks(argv[1]);
  
  return 0;
}
