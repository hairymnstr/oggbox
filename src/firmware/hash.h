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

#ifndef HASH_H
#define HASH_H 1

#include <stdint.h>

#define BUFSIZE (1024 * 1024)

struct md_context {
  uint32_t h[4];
  
  uint8_t digest[16];
  
  uint64_t count;
  
  uint8_t buffer[64];
};

int md5_file(const char *path, uint8_t hash[16]);
int md5_memory(const void *mem, uint64_t len, uint8_t hash[16]);

#endif /* ifndef HASH_H */