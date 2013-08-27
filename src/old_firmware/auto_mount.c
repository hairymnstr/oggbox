#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "auto_mount.h"

void card_plugged() {
  // start from scratch to mount all the partitions
  if(block_init()) {         // sets up the block driver and starts the SD card
    // there was a problem starting the SD card
    return;
  }

  // try and mount the root of the drive
  if(fat_mount(0, block_get_volume_size(), 0)) {
    // mounting the root of the drive as FAT didn't work
    parts = read_partition_table(buffer, block_get_volume_size(), &part_list);

    if(parts > 0) {
      // found at least one real partition
      if(fat_mount(part_list[0].start, part_list[0].length, part_list[0].type)) {
        // failed
        return;
      }
    }
  }
  
  // if we got here then something got mounted
  // check the scan_valid flag to see if this was a boot without a change of card
  if(!scan_valid) {
    scan_media();
    scan_valid = 1;
  }
  // now start the database for media access
  database_init();
}

void card_unplugged() {
  // unmount file systems
  //fat_unmount();
  
  // clear the scan_valid flag
  
}
