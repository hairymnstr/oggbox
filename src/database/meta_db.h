#ifndef META_DB_H
#define META_DB_H 1

//#include "meta.h"

#define METADB_NODE_SIZE 7
#define METADB_MID_PTR 4
#define METADB_KEY_COUNT 3
#define METADB_PASS_KEY 3
#define METADB_2ND_KEY 4

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

uint64_t meta_db_string_hash(char *);

void meta_db_init(struct db_context *);
int meta_db_insert(void *, uint64_t, Node *, int, struct db_context *);
#endif /* ifndef META_DB_H */
