#ifndef META_H
#define META_H 1

#include <stdint.h>

#define META_STR_LEN 64
#define META_MAX_PATH 1024

struct meta {
  char title[META_STR_LEN];
  char artist[META_STR_LEN];
  char album[META_STR_LEN];
  uint32_t track;
  int date;
};

#endif /* ifndef META_H */
