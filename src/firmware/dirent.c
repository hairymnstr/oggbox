#include "sdfat.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include "dirent.h"

extern FileS file_num[];

DIR ret;

DIR *opendir(const char *path) {
  struct stat buffer;
  int fildes;
//   DIR *ret;

//   ret = (DIR *)malloc(sizeof(DIR));

//   fildes = open(path, O_RDWR);
//   iprintf("opendir descriptor %d\n", fildes);
//   if(fstat(fildes, &buffer)) {
//     return NULL;
//   }
//   if(!(buffer.st_mode & S_IFDIR)) {
//     return NULL;
//   }
//   ret.dd_fd = fildes;

  return &ret;
}

struct dirent *readdir(DIR *dirin) {
  if(sdfat_get_next_dirent(dirin->dd_fd - FIRST_DISC_FILENO, &(dirin->dd_dirent))) {
    return NULL;
  } else {
    return &(dirin->dd_dirent);
  }
}

void rewinddir(DIR *dirp) {
  sdfat_select_cluster(dirp->dd_fd - FIRST_DISC_FILENO, 
                 file_num[dirp->dd_fd - FIRST_DISC_FILENO].full_first_cluster);
}

int closedir(DIR *dirp) {
  close(dirp->dd_fd);
}

