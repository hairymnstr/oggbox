#include <stdio.h>
#include <stdlib.h>
#include "fat.h"
#include "block.h"
#include "mbr.h"

int main(int argc, char *argv[]) {
  int p = 0;
  printf("Running FAT tests...\n\n");
  printf("[%4d] start block device emulation...\n", p++);
  block_init();
  
  printf("[%4d] mount filesystem, FAT32\n", p++);
  fat_mount(0, PART_TYPE_FAT32);
  
  printf("[%4d] open a file from the filesystem\n", p++);
  exit(0);
}
