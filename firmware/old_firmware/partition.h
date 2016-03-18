#ifndef PARTITION_H
#define PARTITION_H 1

struct partition {
  blockno_t start;
  blockno_t length;
  uint8_t type;
};

int read_partition_table(uint8_t *mbr, blockno_t volume_size, struct partition **retlist);

#endif /* ifndef PARTITION_H */
