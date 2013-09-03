#include <stdio.h>
#include <string.h>

#include "dirent.h"
#include "playlist.h"

char *playlist_get_next_track() {
  static DIR *dr = NULL;
  struct dirent *de;
  int i;
  int f;
  
  if(dr == NULL) {
    // first time through
    dr = opendir("/");
    f = 0;
  }

  while(1) {
    de = readdir(dr);

    if(de == NULL) {
      if(f > 0) {
        iprintf("rewind\r\n");
        rewinddir(dr);

        f = 0;
      } else {
        iprintf("No ogg files\r\n");
        // there are no .ogg files in this directory, don't
        // keep looking forever
        return NULL;
      } 
    } else {
      i = strlen(de->d_name);
      if(strncmp(&de->d_name[i-4], ".OGG", 4) == 0) {
        iprintf("%s\r\n", de->d_name);
        f ++;
        return de->d_name;
      }
    }
  }
  
  return NULL;
}
