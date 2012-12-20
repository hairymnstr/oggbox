/*
 * This file is part of the oggbox project.
 *
 * Copyright Nathan Dumont 2012 <nathan@nathandumont.com>
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
#include <stdint.h>

#ifndef BLOCK_H
#define BLOCK_H 1

#define BLOCK_SIZE 512

/* block number type is defined here for portability, if you're expecting to have more than 2TB
   with a 512 byte block size you need to use 64bit block numbers */
typedef uint32_t blockno_t;
#define MAX_BLOCK 0xFFFFFFFF

int block_init();
int block_read(blockno_t, void *);
int block_write(blockno_t, void *);
int block_get_size();
int block_get_device_read_only();

#endif /* ifndef BLOCK_H */
