/*
 * This file is part of the oggbox project.
 *
 * Copyright Nathan Dumont 2012 <nathan@nathandumont.com>
 * 
 * Based on the example implementation of the md5 algorithm from Wikipedia
 * http://en.wikipedia.org/wiki/MD5
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "hash.h"

// 32 bit arbitrary offset rotate rather than shift
#define leftrotate(x, c) ((x << c) | (x >> (32-c)))

//r specifies the per-round shift amounts
const uint32_t r[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

//(Or just use the following table):
const uint32_t k[64] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

void md5_transform(struct md_context *context) {
  uint32_t a, b, c, d, f, g, j, x, temp;
  uint32_t buf;
  a = context->h[0];
  b = context->h[1];
  c = context->h[2];
  d = context->h[3];
  
  for(j=0;j<16;j++) {
    f = ((c ^ d) & b) ^ d;
    g = j;
    temp = d;
    d = c;
    c = b;
    memcpy(&buf, &context->buffer[g*4], sizeof(uint32_t));
    x = (a + f + k[j] + buf);//(*(uint32_t *)(&context->buffer[g * 4])));
    b = b + leftrotate(x, r[j]);
    a = temp;
  }
  for(j=16;j<32;j++) {
    f = ((b ^ c) & d) ^ c;
    g = (5*j + 1) % 16;
    temp = d;
    d = c;
    c = b;
    memcpy(&buf, &context->buffer[g*4], sizeof(uint32_t));
    x = (a + f + k[j] + buf);//(*(uint32_t *)(&context->buffer[g * 4])));
    b = b + leftrotate(x, r[j]);
    a = temp;
  }
  for(j=32;j<48;j++) {
    f = b ^ c ^ d;
    g = (3 * j + 5) % 16;
    temp = d;
    d = c;
    c = b;
    memcpy(&buf, &context->buffer[g*4], sizeof(uint32_t));
    x = (a + f + k[j] + buf);//(*(uint32_t *)(&context->buffer[g * 4])));
    b = b + leftrotate(x, r[j]);
    a = temp;
  }
  for(j=48;j<64;j++) {
    f = c ^ (b | (~d));
    g = (7*j) % 16;
    temp = d;
    d = c;
    c = b;
    memcpy(&buf, &context->buffer[g*4], sizeof(uint32_t));
    x = (a + f + k[j] + buf);//(*(uint32_t *)(&context->buffer[g * 4])));
    b = b + leftrotate(x, r[j]);
    a = temp;
  }
  context->h[0] += a;
  context->h[1] += b;
  context->h[2] += c;
  context->h[3] += d;
}

void md5_start(struct md_context *context) {
  context->h[0] = 0x67452301;
  context->h[1] = 0xefcdab89;
  context->h[2] = 0x98badcfe;
  context->h[3] = 0x10325476;
  
  context->count = 0;
  
  memset(context->buffer, 0, 64);
  
  return;
}

void md5_update(struct md_context *context, uint8_t *chunk, uint64_t chunk_size) {
  int buflen = context->count & 63;
  context->count += chunk_size;
  
  if((buflen + chunk_size) < 64) {
    memcpy(&context->buffer[buflen], chunk, chunk_size);
    return;
  }
  
  memcpy(&context->buffer[buflen], chunk, 64 - buflen);
  md5_transform(context);
  chunk_size -= (64 - buflen);
  chunk += (64 - buflen);
  while(chunk_size >= 64) {
    memcpy(context->buffer, chunk, 64);
    md5_transform(context);
    chunk_size -= 64;
    chunk += 64;
  }
  memcpy(context->buffer, chunk, chunk_size);
  return;
}

void md5_finish(struct md_context *context) {
  int buflen = context->count & 63;
  uint64_t bitcount;
  
  context->buffer[buflen++] = 0x80;
  memset(&context->buffer[buflen], 0, 64-buflen);
  if(buflen > 56) {
    md5_transform(context);
    memset(context->buffer, 0, 64);
  }
//   *(uint64_t *)(&context->buffer[56]) = context->count * 8;
  bitcount = context->count * 8;
  memcpy(&context->buffer[56], &bitcount, sizeof(uint64_t));
  md5_transform(context);
  memcpy(context->digest, context->h, 16);
  return;
}

int md5_file(const char *path, uint8_t hash[16]) {
  uint8_t *buffer;
  uint32_t l;
  FILE *fp;
  
  struct md_context context;
  
  buffer = (uint8_t *)malloc(BUFSIZE);
  if(!(fp = fopen(path, "rb"))) {
    fprintf(stderr, "File couldn't be read.\n");
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  md5_start(&context);
  
  l = fread(buffer, 1, BUFSIZE, fp);
  while(l == BUFSIZE) {
    md5_update(&context, buffer, BUFSIZE);
    l = fread(buffer, 1, BUFSIZE, fp);
  }
  md5_update(&context, buffer, l);
  
  md5_finish(&context);
  
  memcpy(hash, context.digest, 16);
  return 0;
}

int md5_memory(const void *mem, uint64_t len, uint8_t hash[16]) {
  uint8_t *buffer;
  uint64_t offs = 0;
//   uint32_t l=0;
  struct md_context context;
  
//   printf("hashing %ld bytes.\n", len);
  
  buffer = (uint8_t *)malloc(BUFSIZE);
  
  md5_start(&context);
  
  while(len > BUFSIZE) {
    memcpy(buffer, mem+offs, BUFSIZE);
    md5_update(&context, buffer, BUFSIZE);
    len -= BUFSIZE;
    offs += BUFSIZE;
  }
  
  memcpy(buffer, mem+offs, len);
  md5_update(&context, buffer, len);
  md5_finish(&context);
  memcpy(hash, context.digest, 16);
  return 0;
}
