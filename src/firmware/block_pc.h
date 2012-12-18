#ifndef BLOCK_PC_H
#define BLOCK_PC_H 1

int block_pc_snapshot(const char *filename, uint64_t start, uint64_t len);
int block_pc_snapshot_all(const char *filename);
int block_pc_hash(uint64_t start, uint64_t len, uint8_t hash[16]);
int block_pc_hash_all(uint8_t hash[16]);

#endif /* ifndef BLOCK_PC_H */
