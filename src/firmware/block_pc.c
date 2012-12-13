#include <stdio.h>
#include <stdlib.h>
#include "block.h"

FILE *block_fp=NULL;
uint64_t block_fs_size=0;

int block_init() {
  if(!(fp = fopen("filesystem.img", "rb+"))) {
    return -1;
  }
  fseek(fp, 0, SEEK_END);
  block_fs_size = ftell(fp);
  return 0;
}

int block_read(blockno_t block, void *buffer) {
  // we can't allow the file to grow (wouldn't happen with a physical volume) so need to check
  // first because in rb+ file will grow if we seek past the end.
  if((block+1) * BLOCK_SIZE - 1 > block_fs_size) {
    return -1;
  }
  fseek(block_fp, block * BLOCK_SIZE, SEEK_SET);
  if(fread(buffer, 1, BLOCK_SIZE, block_fp) < BLOCK_SIZE) {
    return -1;
  }
  fflush(fp);
  return 0;
}

int block_write(blockno_t block, void *buffer) {
  if((block + 1) * BLOCK_SIZE - 1 > block_fs_size) {
    return -1;
  }
  
  fseek(block_fp, block * BLOCK_SIZE, SEEK_SET);
  if(fwrite(buffer, 1, BLOCK_SIZE, block_fp) < BLOCK_SIZE) {
    return -1;
  }
  fflush(fp);
  return 0;
}

int block_get_size() {
  return BLOCK_SIZE;
}
