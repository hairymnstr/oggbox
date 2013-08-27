#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "hash.h"

int main(int argc, char *argv[]) {
  FILE *fp;
  uint8_t *buffer;
  uint64_t flen;
  uint8_t hash[16];
  
  if(argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(-2);
  }

  if(!(fp = fopen(argv[1], "rb"))) {
    printf("Couldn't open file: %s\n", strerror(errno));
    exit(-1);
  }

  fseek(fp, 0, SEEK_END);
  flen = ftell(fp);
  if(flen > (2048L * 1024L * 1024L)) {
    printf("Aborting, the file is too large.\n");
    exit(-1);
  }

  buffer = (uint8_t *)malloc(flen);

  fseek(fp, 0, SEEK_SET);

  fread(buffer, 1, flen, fp);
//   printf("Reading %ld bytes, got %ld bytes.\n", flen, fread(buffer, 1, flen, fp));

  fclose(fp);

  md5_memory(buffer, flen, hash);

  for(flen=0;flen<16;flen++) {
    printf("%02x", hash[flen]);
  }
  printf("\n");

  free(buffer);
  
  md5_file(argv[1], hash);
  for(flen=0;flen<16;flen++) {
    printf("%02x", hash[flen]);
  }
  printf("\n");

  exit(0);
}
