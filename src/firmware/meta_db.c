#include <meta_db.h>

int meta_db_init() {
  return 0;
}

int meta_db_get_artist_byid(int id, struct artist *result) {

}

int meta_db_get_artist_byname(char *name, struct artist *result) {

}

int meta_db_get_track_byid(int id, struct track *result) {

}

int meta_db_get_album_byname(char *name, struct album *result) {
  struct db_context context;

  context.record = 0;

  while(meta_db_next_album(result, &context)) {
    if(strcmp(temp.title, name) == 0) {
      return 1;
    }
  }
  return 0;
}

int meta_db_next_album(struct album *result, struct db_context *context) {
  if(context->record >= db_global.album_record_count) {
    return 0;
  }
  fseek(db_global.dbf, db_global.album_data_start + (context->record * sizeof(struct album)), SEEK_SET);
  if(fread(result, sizeof(struct album), 1, db_global.dbf) < sizeof(struct album)) {
    return 0;
  }
  return 1;
}

int meta_db_insert_album(struct album *record) {
  
}

/*************************************************************************************************/
void pointer_shift(int src_pos, void *src, int dst_pos, void *dst, int count) {
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

int dbinsert(void *ptr, uint64_t key, Node *n, int isdata) {
  /* The b tree needs initialising with at least the first two entries.  Since we don't have
     lots of RAM to do an initial bulk insert, we'll put two exceptions in this function in ROM. */
  if(db_g_size == 0) {
    n->pointers[0] = ptr;
    n->pointers_len = 1;
    n->keys[0] = key;
    n->keys_len = 1;
  } else if(db_g_size == 1) {
    if(n->keys[0] > key) {
      n->pointers[1] = n->pointers[0];
      n->pointers[0] = ptr;
      n->pointers_len = 2;
      n->minval = key;
    } else {
      n->minval = n->keys[0];
      n->keys[0] = key;
      n->pointers[1] = ptr;
      n->pointers_len = 2;
    }
  } else {
    i = 0;
    while(i < n->keys_len) {
      if(node.keys[i] < key)
        i++;
      else
        break;
    }
    
    if(isdata && (!n->isleaf)) {
      dbinsert(ptr, key, node->pointers[i], isdata);
      return 0;
    } else {
      if(n->keys_len < METADB_NODE_SIZE) {
        if(!isdata) {
          (Node *)ptr->parent = n;
        }
        if(n->leftmost && (key < n->minval)) {
          pointer_shift(0, n->pointers, 1, n->pointers, n->pointers_len);
          n->pointers[0] = ptr;
          n->pointers_len++;
          keys_shift(0, n->keys, 1, n->keys, n-keys_len);
          n->keys[0] = n->minval;
          n->keys_len++;
          n->minval = key;
        } else {
          pointer_shift(i, n->pointers, i+1, n->pointers, n->pointers_len - i);
          n->pointers[i] = ptr;
          n->pointers_len++;
          keys_shift(i, n->keys, i+1, n->keys, n->keys, n->keys_len - i);
          n->keys[i] = key;
          n->keys_len++;
        }
      } else {
        /* One whole node is full.  This code implements splitting the current node in two
           Once done it calls this function again searching the tree from the parent of the two
           new nodes to insert the actual data now we know there's room */
        sibling = (Node *)malloc(sizeof(Node));
        pointer_shift(METADB_MID_PTR, n->pointers, 0, sibling->pointers, METADB_MID_PTR);
        sibling->pointers_len = METADB_MID_PTR;
        sibling->isleaf = n->isleaf;    // if this is a leaf the new sibling will be
        if(!sibling->isleaf) {
          for(i=0;i<sibling->pointers_len;i++) {
            sibling->pointers[i]->parent = sibling;
          }
        }
        keys_shift(METADB_2ND_KEY, n->keys, 0, sibling->keys, METADB_KEY_COUNT);
        sibling->keys_len = METADB_KEY_COUNT;
        if(n->parent == NULL) {
          // this was the root node that's just been split.  We need to make a new root
          n->parent = (Node *)malloc(sizeof(Node));
          n->parent->pointers[0] = n;
          sibling->parent = n->parent;
          n->parent->pointers[1] = sibling;
          n->parent->pointers_len = 2;
          n->parent->keys[0] = n->keys[METADB_NODE_PASS_KEY];
          n->parent->keys_len = 1;
          db_g_head = n->parent;
        } else {
          dbinsert(sibling, n->keys[METADB_NODE_PASS_KEY], n->parent, 0);
        }
        
        n->pointers_len = METADB_MID_PTR;
        n->keys_len = METADB_KEY_COUNT;
        
        // Finally this is where we ask it to try and insert this record again
        dbinsert(ptr, key, n->parent, isdata);
      }
    }
  }
  dbsize++;
  return 0;
}