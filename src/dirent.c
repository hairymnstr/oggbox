#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "fat.h"
#include "dirent.h"

extern FileS file_num[];

DIR *opendir(const char *path) {
  static DIR ret;
  struct stat buffer;
  int rerrno;
  int fildes;

  fildes = open(path, O_RDONLY);
  if(fstat(fildes, &buffer)) {
    return NULL;
  }
  if(!(buffer.st_mode & S_IFDIR)) {
    close(fildes);
    return NULL;
  }
  ret.dd_fd = fildes;

  return &ret;
}

struct dirent *readdir(DIR *dirin) {
  if(fat_get_next_dirent(dirin->dd_fd - FIRST_DISC_FILENO, &(dirin->dd_dirent))) {
    return NULL;
  } else {
    return &(dirin->dd_dirent);
  }
}

void rewinddir(DIR *dirp) {
  fat_select_cluster(dirp->dd_fd - FIRST_DISC_FILENO, 
                 file_num[dirp->dd_fd - FIRST_DISC_FILENO].full_first_cluster);
}

int closedir(DIR *dirp) {
  int rerrno;
  return close(dirp->dd_fd);
}

