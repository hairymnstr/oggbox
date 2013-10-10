#ifndef BTREE_H
#define BTREE_H 1

#define METADB_NODE_SIZE 7
#define METADB_MID_PTR 4
#define METADB_KEY_COUNT 3
#define METADB_PASS_KEY 3
#define METADB_2ND_KEY 4

#define CACHE_NODE_COUNT 40

typedef struct Snode {
  uint32_t pointers[METADB_NODE_SIZE+1];
  int pointers_len;
  uint64_t keys[METADB_NODE_SIZE];
  int keys_len;
  int32_t parent;
  int leftmost;
  uint64_t minval;
  int depth;
  int32_t sibling_next;
  int32_t sibling_prev;
} Node;

struct cache_entry {
  int dirty;
  int lru;
  uint32_t id;
  Node node;
};

struct cache_context {
  int node_count;
  int fd;
  int lru_val;
  int size;
  struct cache_entry *entries;
};

struct db_context {
  uint32_t head;
  int size;
  uint32_t nid;
  uint32_t pid;
  struct cache_context cache;
};

void cache_init(const char *cache_file, int cache_size, struct cache_context *node_cache);
int cache_save_node(uint32_t node_id, Node *input, struct cache_context *node_cache);
int cache_get_node(uint32_t node_id, Node *output, struct cache_context *node_cache);

void btree_init(char *cache_file, int cache_size, struct db_context *);
int btree_insert(uint32_t, uint64_t, uint32_t, int, struct db_context *);
int btree_lookup(uint64_t sought, uint32_t *sought_id, int row, struct db_context *context);
int btree_walk(uint32_t *next_id, struct db_context *);
void btree_rewind(struct db_context *);

#endif /* ifndef BTREE_H */
