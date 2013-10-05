#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/unistd.h>

#include "btree.h"

#define pvPortMalloc malloc

/*************************************************************************************************/
/* Node cache system                                                                             */
/*************************************************************************************************/

void cache_init(const char *cache_file, int cache_size, struct cache_context *node_cache) {
  int i;
  
  node_cache->size = cache_size;
  
  node_cache->entries = (struct cache_entry *)pvPortMalloc(sizeof(struct cache_entry) * cache_size);
  for(i=0;i<node_cache->size;i++) {
    node_cache->entries[i].lru = 0;      // smaller = older
    node_cache->entries[i].dirty = 0;    // if its not dirty it won't try and write
    node_cache->entries[i].id = -1;      // will never match an unsigned id
  }
  node_cache->lru_val = 0;
  node_cache->node_count = 0;
  node_cache->fd = open(cache_file, O_RDWR | O_CREAT, 0777);
}

uint32_t cache_new_node(struct cache_context *node_cache) {
//   printf("New node %d\r\n", node_cache.node_count);
  return node_cache->node_count++;
}

int cache_save_node(uint32_t node_id, Node *node, struct cache_context *node_cache) {
  int i;
  int lru_id;
  int lru_val;
  
  if(node_id >= node_cache->node_count) {
    return -1;
  }
  for(i=0;i<node_cache->size;i++) {
    if(node_cache->entries[i].id == node_id) {
      memcpy(&node_cache->entries[i].node, node, sizeof(Node));
      node_cache->entries[i].dirty = 1;
      node_cache->entries[i].lru = node_cache->lru_val++;
      return 0;
    }
  }
  // need to check for empty cache entries
  lru_val = node_cache->entries[0].lru;
  lru_id = 0;
  for(i=1;i<node_cache->size;i++) {
    if(node_cache->entries[i].lru < lru_val) {
      lru_val = node_cache->entries[i].lru;
      lru_id = i;
    }
  }
  
  if(node_cache->entries[lru_id].dirty) {
    lseek(node_cache->fd, sizeof(Node) * node_cache->entries[lru_id].id, SEEK_SET);
    write(node_cache->fd, &node_cache->entries[lru_id].node, sizeof(Node));
  }
  
  node_cache->entries[lru_id].dirty = 1;
  node_cache->entries[lru_id].lru = node_cache->lru_val++;
  node_cache->entries[lru_id].id = node_id;
  memcpy(&node_cache->entries[lru_id].node, node, sizeof(Node));
  
  return 0;
}

int cache_get_node(uint32_t node_id, Node *node, struct cache_context *node_cache) {
  int i;
  int lru_id;
  int lru_val;
  
  if(node_id >= node_cache->node_count) {
    return -1;  // node id isn't in the database
  }
  for(i=0;i<node_cache->size;i++) {
    if(node_cache->entries[i].id == node_id) {
      memcpy(node, &node_cache->entries[i].node, sizeof(Node));
      node_cache->entries[i].lru = node_cache->lru_val++;
      return 0;
    }
  }
  // not a cached node, load it
  // find the least recently used node
  lru_val = node_cache->entries[0].lru;
  lru_id = 0;
  for(i=1;i<node_cache->size;i++) {
    if(node_cache->entries[i].lru < lru_val) {
      lru_val = node_cache->entries[i].lru;
      lru_id = i;
    }
  }
  
  if(node_cache->entries[lru_id].dirty) {
    lseek(node_cache->fd, sizeof(Node) * node_cache->entries[lru_id].id, SEEK_SET);
    write(node_cache->fd, &node_cache->entries[lru_id].node, sizeof(Node));
  }
  // now load the node into cache
  lseek(node_cache->fd, sizeof(Node) * node_id, SEEK_SET);
  read(node_cache->fd, &node_cache->entries[lru_id].node, sizeof(Node));
  
  node_cache->entries[lru_id].id = node_id;
  node_cache->entries[lru_id].lru = node_cache->lru_val++;
  node_cache->entries[lru_id].dirty = 0;
  memcpy(node, &node_cache->entries[lru_id].node, sizeof(Node));
  
  return 0;
}

/*************************************************************************************************/
/* Generic B-Tree database with minimal memory footprint functions below                         */
/*************************************************************************************************/

void btree_init(char *cache_file, int cache_size, struct db_context *c) {
  Node head;
  cache_init(cache_file, cache_size, &c->cache);
  
  c->head = cache_new_node(&c->cache);
  head.pointers_len = 0;
  head.keys_len = 0;
  head.parent = -1;
  head.isleaf = 1;
  head.leftmost = -1;
  head.minval = -1;
  head.depth = 0;
  cache_save_node(c->head, &head, &c->cache);
  
  c->size = 0;
}

void pointer_shift(int src_pos, uint32_t *src, int dst_pos, uint32_t *dst, int count) {
  while(count > 0) {
    dst[count + dst_pos - 1] = src[count + src_pos - 1];
    count--;
  }
}

void keys_shift(int src_pos, uint64_t *src, int dst_pos, uint64_t *dst, int count) {
  while(count > 0) {
    dst[count + dst_pos - 1] = src[count + src_pos - 1];
    count--;
  }
}

int btree_insert(uint32_t ptr, uint64_t key, uint32_t nid, int isdata, struct db_context *context) {
  int i;
  uint32_t sibling_id;
  Node n, child, sibling;
//   char msg[9];
//   int j;
  
//   for(j=7;j>-1;j--) {
//     msg[j] = (key >> (8 * (7-j))) & 0xff;
//   }
//   msg[8] = 0;
  
//   printf("btree_insert(%u, %s, %u, %d, context);\r\n", ptr, msg, nid, isdata);
  
  cache_get_node(nid, &n, &context->cache);
  /* The b tree needs initialising with at least the first two entries.  Since we don't have
     lots of RAM to do an initial bulk insert, we'll put two exceptions in this function in ROM. */
  if(context->size == 0) {
    n.pointers[0] = ptr;
    n.pointers_len = 1;
    n.keys[0] = key;
    n.keys_len = 1;
    context->size++;
    cache_save_node(nid, &n, &context->cache);
    return 0;
  } else if(context->size == 1) {
    if(n.keys[0] > key) {
      n.pointers[1] = n.pointers[0];
      n.pointers[0] = ptr;
      n.pointers_len = 2;
      n.minval = key;
    } else {
      n.minval = n.keys[0];
      n.keys[0] = key;
      n.pointers[1] = ptr;
      n.pointers_len = 2;
    }
    context->size++;
    cache_save_node(nid, &n, &context->cache);
    return 0;
  } else {
    i = 0;
    while(i < n.keys_len) {
      if(n.keys[i] < key)
        i++;
      else
        break;
    }
//     printf("n.keys[i] = %ld, key = %ld, nid=%d\r\n", n.keys[i], key, nid);
    if(isdata < n.depth) {
//       printf("Descend\r\n");
      btree_insert(ptr, key, n.pointers[i], isdata, context);
      return 0;
    } else {
      if(n.keys_len < METADB_NODE_SIZE) {
//         printf("just store.\r\n");
        if(isdata) {
          cache_get_node(ptr, &child, &context->cache);
          child.parent = nid;
          cache_save_node(ptr, &child, &context->cache);
        }
        if(n.leftmost && (key < n.minval)) {
          pointer_shift(0, n.pointers, 1, n.pointers, n.pointers_len);
          n.pointers[0] = ptr;
          n.pointers_len++;
          keys_shift(0, n.keys, 1, n.keys, n.keys_len);
          n.keys[0] = n.minval;
          n.keys_len++;
          n.minval = key;
          cache_save_node(nid, &n, &context->cache);
          // propagate the new minval back up to root
          while(n.parent != -1) {
            i = n.parent;
            cache_get_node(i, &n, &context->cache);
            n.minval = key;
            cache_save_node(i, &n, &context->cache);
          }
        } else {
          pointer_shift(i+1, n.pointers, i+2, n.pointers, n.pointers_len - i - 1);
          n.pointers[i+1] = ptr;
          n.pointers_len++;
          keys_shift(i, n.keys, i+1, n.keys, n.keys_len - i);
          n.keys[i] = key;
          n.keys_len++;
          cache_save_node(nid, &n, &context->cache);
        }
        if(!isdata) {
          context->size++;
        }
        return 0;
      } else {
//         printf("Full node %d\r\n", nid);
        /* One whole node is full.  This code implements splitting the current node in two
           Once done it calls this function again searching the tree from the parent of the two
           new nodes to insert the actual data now we know there's room */
        sibling_id = cache_new_node(&context->cache);
        pointer_shift(METADB_MID_PTR, n.pointers, 0, sibling.pointers, METADB_MID_PTR);
        sibling.pointers_len = METADB_MID_PTR;
        n.pointers_len = METADB_MID_PTR;
        sibling.isleaf = n.isleaf;    // if this is a leaf the new sibling will be
        if(!sibling.isleaf) {
          for(i=0;i<sibling.pointers_len;i++) {
            cache_get_node(sibling.pointers[i], &child, &context->cache);
            child.parent = sibling_id;
            cache_save_node(sibling.pointers[i], &child, &context->cache);
          }
        }
        keys_shift(METADB_2ND_KEY, n.keys, 0, sibling.keys, METADB_KEY_COUNT);
        sibling.keys_len = METADB_KEY_COUNT;
        n.keys_len = METADB_KEY_COUNT;
        sibling.leftmost = 0;
        sibling.depth = n.depth;
        cache_save_node(sibling_id, &sibling, &context->cache);
        cache_save_node(nid, &n, &context->cache);
        if(n.parent == -1) {
          // this was the root node that's just been split.  We need to make a new root
          // reuse child
          n.parent = cache_new_node(&context->cache);
          cache_save_node(nid, &n, &context->cache);
          child.parent = -1;
          child.isleaf = 0;
          child.leftmost = -1;
          child.pointers[0] = nid;
          sibling.parent = n.parent;
          cache_save_node(sibling_id, &sibling, &context->cache);
          child.pointers[1] = sibling_id;
          child.pointers_len = 2;
          child.keys[0] = n.keys[METADB_PASS_KEY];
          child.keys_len = 1;
          child.minval = n.minval;
          child.depth = n.depth + 1;
          context->head = n.parent;
          cache_save_node(n.parent, &child, &context->cache);
        } else {
          btree_insert(sibling_id, n.keys[METADB_PASS_KEY], context->head, n.depth+1, context);
        }
        
        // Finally this is where we ask it to try and insert this record again
        btree_insert(ptr, key, context->head, isdata, context);
      }
    }
  }
  return 0;
}

int btree_lookup_recurse(uint64_t sought, uint32_t *sought_id, uint32_t start, struct db_context *context) {
  Node n;
  int i;
  
  cache_get_node(start, &n, &context->cache);
  for(i=0;i<n.keys_len;i++) {
    if(sought <= n.keys[i]) {
      break;
    }
  }
  if(n.isleaf) {
    if(n.keys[i] == sought) {
      *sought_id = n.pointers[i];
      return 1;
    } else {
      return 0;
    }
  } else {
    return btree_lookup_recurse(sought, sought_id, start, context);
  }
}

int btree_lookup(uint64_t sought, uint32_t *sought_id, struct db_context *context) {
  return btree_lookup_recurse(sought, sought_id, context->head, context);
}

int btree_walk(uint32_t *next_id, struct db_context *context) {
  Node n;
  uint32_t temp;
  int i;
  
  cache_get_node(context->nid, &n, &context->cache);
  if(context->pid < n.pointers_len) {
    *next_id = n.pointers[context->pid++];
//     printf("Track %d\r\n", *next_id);
    return 1;
  } else {
    while(n.parent != -1) {
      temp = n.parent;
      cache_get_node(n.parent, &n, &context->cache);
      for(i=0;i<n.pointers_len;i++) {
        if(n.pointers[i] == context->nid) {
          break;
        }
      }
      
//     printf("pl: %d, parent: %d, nid: %d\r\n", n.pointers_len, n.parent, context->nid);
    
//       printf("Found old node at index %d of %d\r\n", i, n.pointers_len);
      if(i+1 < n.pointers_len) {
        context->nid = n.pointers[i+1];
        cache_get_node(context->nid, &n, &context->cache);
        while(!n.isleaf) {
          context->nid = n.pointers[0];
          cache_get_node(context->nid, &n, &context->cache);
        }
        *next_id = n.pointers[0];
        context->pid = 1;
        return 1;
      } else {
        context->nid = temp;
      }
    }
  }
  return 0;
}

#ifdef INTEGRITY_CHECKS
int btree_walk2(uint64_t *next_id, struct db_context *context) {
  Node n;
  uint32_t temp;
  int i;
  
  cache_get_node(context->nid, &n);
  if(context->pid < n.keys_len) {
    *next_id = n.keys[context->pid++];
    return 1;
  } else {
    while(n.parent != -1) {
      temp = n.parent;
      cache_get_node(n.parent, &n);
      for(i=0;i<n.pointers_len;i++) {
        if(n.pointers[i] == context->nid) {
          break;
        }
      }
      
//     printf("pl: %d, parent: %d, nid: %d\r\n", n.pointers_len, n.parent, context->nid);
    
//       printf("Found old node at index %d of %d\r\n", i, n.pointers_len);
      if(i+1 < n.pointers_len) {
        context->nid = n.pointers[i+1];
        cache_get_node(context->nid, &n);
        while(!n.isleaf) {
          context->nid = n.pointers[0];
          cache_get_node(context->nid, &n);
        }
        *next_id = n.keys[0];
        context->pid = 1;
        return 1;
      } else {
        context->nid = temp;
      }
    }
  }
  return 0;
}
#endif

void btree_rewind(struct db_context *context) {
  Node n;
  
  context->nid = context->head;
  context->pid = 0;
  
  cache_get_node(context->nid, &n, &context->cache);
  while(!n.isleaf) {
    context->nid = n.pointers[0];
    cache_get_node(context->nid, &n, &context->cache);
  }
//   printf("nid: %d, %d\r\n", context->nid, n.isleaf);
  return;
}

#ifdef INTEGRITY_CHECKS
int btree_check(struct db_context *context) {
  uint64_t check = 0;
  uint64_t old_check = 0;
  char msg[9];
  int j;
  
  btree_rewind(context);
  while(btree_walk2(&check, context)) {
    if(check < old_check) {
        for(j=7;j>-1;j--) {
          msg[j] = (check >> (8 * (7-j))) & 0xff;
        }
        msg[8] = 0;
        printf("old_check %s, ", msg);
        
        for(j=7;j>-1;j--) {
          msg[j] = (old_check >> (8 * (7-j))) & 0xff;
        }
        msg[8] = 0;
        printf("check %s\r\n", msg);
      
      return 1;
    }
    old_check = check;
  }
  return 0;
}

int btree_validate(struct db_context *context) {
  Node n, m;
  int i, j;
  
  cache_get_node(context->head, &n);
  if(n.parent == -1) {
    printf("Root node is valid (head = %d)\r\n", context->head);
  } else {
    printf("Failed: root parent check.\r\n");
    return -1;
  }
  
  for(i=0;i<node_cache.node_count;i++) {
    if(i != context->head) {
      cache_get_node(i, &n);
      if(n.parent == -1) {
        printf("Multiple Root nodes!!\r\n");
        return -1;
      }
    }
  }
  
  for(i=0;i<node_cache.node_count;i++) {
    cache_get_node(i, &n);
    if(!n.isleaf) {
      for(j=0;j<n.pointers_len;j++) {
        cache_get_node(n.pointers[j], &m);
        if(m.parent != i) {
          printf("Integrity check failed on node %d\r\n", i);
          printf("Child %d points at %d\r\n", j, n.pointers[j]);
          printf("%d's parent is %d\r\n", n.pointers[j], m.parent);
          return -1;
        }
      }
    }
  }
  return 0; 
}

void dump_leaves(struct db_context *context) {
  Node n;
  int i, j;
  for(i=0;i<node_cache.node_count;i++) {
    cache_get_node(i, &n);
    if(n.isleaf) {
      for(j=0;j<n.keys_len;j++) {
        printf("%ld\r\n", n.keys[j]);
      }
      printf("----------------------\r\n");
    }
  }
}

void dbgoutput(FILE *fw, uint64_t item,  uint32_t head) {
  char job_names[5000][50];
  uint32_t job_pointers[5000];
  int job_front = 0;
  int job_end = 1;
  int i, j;
  char msg[9];
  Node n;
  
  sprintf(job_names[0], "Root (%u)", head);
  job_pointers[0] = head;
  
  fprintf(fw, "Store %lu\n", item);
  while(job_front != job_end) {
    fprintf(fw, "<h2 id='table%d'>%s</h2>\n", job_pointers[job_front], job_names[job_front]);
    cache_get_node(job_pointers[job_front], &n);
    fprintf(fw, "<h3><a href='#table%d'>Parent: %u</a></h3>\n", n.parent, n.parent);
    fprintf(fw, "<table>\n");
    fprintf(fw, "<tr>");
    fprintf(fw, "<td></td>");
    for(i=0;i<METADB_NODE_SIZE;i++) {
      if(n.keys_len > i) {
        for(j=7;j>-1;j--) {
          msg[j] = (n.keys[i] >> (8 * (7-j))) & 0xff;
        }
        msg[8] = 0;
        fprintf(fw, "<td>%s</td><td></td>", msg);
      } else
        fprintf(fw, "<td></td><td></td>");
    }
    fprintf(fw, "</tr>\n<tr>");
    
    for(i=0;i<METADB_NODE_SIZE+1;i++) {
      if(n.pointers_len > i) {
        if(n.isleaf) {
          fprintf(fw, "<td>%u</td><td></td>", n.pointers[i]);
        } else {
          fprintf(fw, "<td><a href='#table%d'>Table %d</a></td><td></td>", n.pointers[i], n.pointers[i]);
          sprintf(job_names[job_end], "Table %d", n.pointers[i]);
          job_pointers[job_end++] = n.pointers[i];
        }
      } else {
        fprintf(fw, "<td></td><td></td>");
      }
    }
    fprintf(fw, "</tr>\n<tr>");
    
    for(i=0;i<METADB_NODE_SIZE+1;i++) {
      if(n.pointers_len > i) {
        if(n.isleaf) {
          fprintf(fw, "<td>%u</td><td></td>", (n.pointers[i]));
        } else {
          fprintf(fw, "<td></td><td></td>");
        }
      } else {
        fprintf(fw, "<td></td><td></td>");
      }
    }
    fprintf(fw, "</tr>");
    fprintf(fw, "</table>");
    
    job_front++;
    
  }
  
  fprintf(fw, "<hr/>\n\n");
  
  return;
}
#endif
