#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "hash.h"
#include "block.h"
#include "block_pc.h"

uint64_t block_fs_size=0;
uint8_t *blocks = NULL;
int block_ro;

int block_init() {
  FILE *block_fp;
  if(!(block_fp = fopen("partitioned.img", "rb"))) {
    return -1;
  }
  fseek(block_fp, 0, SEEK_END);
  block_fs_size = ftell(block_fp);
  if(!(block_fs_size < 2048L * 1024L * 1024L)) {
    printf("Aborting, image is over 2GB.\n");
    exit(-1);
  }
  blocks = (uint8_t *)malloc(sizeof(uint8_t) * block_fs_size);
  
  fseek(block_fp, 0, SEEK_SET);
  fread(blocks, 1, block_fs_size, block_fp);
  
  fclose(block_fp);
  return 0;
}

int block_read(blockno_t block, void *buffer) {
  /* we can't allow the file to grow (wouldn't happen with a physical volume) so need to check
     first because in rb+ file will grow if we seek past the end. */
  if((block+1) * BLOCK_SIZE - 1 > block_fs_size) {
    return -1;
  }
//   fseek(block_fp, block * BLOCK_SIZE, SEEK_SET);
//   if(fread(buffer, 1, BLOCK_SIZE, block_fp) < BLOCK_SIZE) {
//     return -1;
//   }
//   fflush(block_fp);
  memcpy(buffer, blocks + block * BLOCK_SIZE, BLOCK_SIZE);
  return 0;
}

int block_write(blockno_t block, void *buffer) {
  if((block + 1) * BLOCK_SIZE - 1 > block_fs_size) {
    return -1;
  }
  
//   fseek(block_fp, block * BLOCK_SIZE, SEEK_SET);
//   if(fwrite(buffer, 1, BLOCK_SIZE, block_fp) < BLOCK_SIZE) {
//     return -1;
//   }
//   fflush(block_fp);
  memcpy(blocks + block * BLOCK_SIZE, buffer, BLOCK_SIZE);
  return 0;
}

int block_get_volume_size() {
  return block_fs_size / BLOCK_SIZE;
}

int block_get_block_size() {
  return BLOCK_SIZE;
}

int block_get_device_read_only() {
  return block_ro;
}

void block_pc_set_ro() {
  block_ro = -1;
}

void block_pc_set_rw() {
  block_ro = 0;
}

int block_pc_snapshot(const char *filename, uint64_t start, uint64_t len) {
  FILE *fp;
  
  if(!(fp = fopen(filename, "wb"))) {
    return -1;
  }
  
  fwrite(blocks + start, 1, len, fp);
  
  fclose(fp);
  
  return 0;
}

int block_pc_snapshot_all(const char *filename) {
  return block_pc_snapshot(filename, 0, block_fs_size);
}

int block_pc_hash(uint64_t start, uint64_t len, uint8_t hash[16]) {
  return md5_memory(&blocks[start], len , hash);
}

int block_pc_hash_all(uint8_t hash[16]) {
  return md5_memory(blocks, block_fs_size, hash);
}
