#ifndef META_DB_H
#define META_DB_H 1

#include <meta.h>

struct album {
  int id;
  char title[META_STR_LEN];
  int year;
};

struct artist {
  int id;
  char name[META_STR_LEN];
};

struct track {
  int id;
  int album;
  int artist;
  int trackno;
  char title[META_STR_LEN];
};

#endif /* ifndef META_DB_H */
