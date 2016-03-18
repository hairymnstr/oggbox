#include <stdint.h>
#include <stdio.h>
#include "block.h"
#include "mbr.h"
#include "partition.h"

int read_partition_table(uint8_t *mbr, blockno_t volume_size, struct partition **retlist) {
  static struct partition retval[4];
  mbr_entry *entry = (mbr_entry *)(mbr + PARTITION0START);
  int i, j=0;
  iprintf("Start: 0, Length: %u\r\n", (unsigned int)volume_size);
  for(i=0;i<4;i++) {
    // validate this partition entry
    iprintf("Start: %u, Length: %u, Type: %02x\r\n", (unsigned int)entry->lba_start, (unsigned int)entry->length, (unsigned int)entry->type);
    if((entry->lba_start < volume_size) && 
       ((entry->lba_start + entry->length) <= volume_size) &&
       (entry->lba_start > 0) &&
       (entry->length > 0)) {
      // the partion is non-zero length and smaller than the disk.
      // no guarantee the partition isn't overlapping another
      retval[j].start = entry->lba_start;
      retval[j].type = entry->type;
      retval[j].length = entry->length;
      j++;
    }
    entry += 1;
  }
  
  *retlist = retval;
  return j;
}
