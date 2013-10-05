#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include "dirent.h"
#include "ogg_meta.h"
#include "btree.h"
#include "meta_db.h"

#define META_PATH_LEN 256
#define MAX_NESTED_DIRS 6
#define DB_INDEX_RAM_COUNT 1024
#define DB_CACHE_FILE_NAME_LEN 128
#define MAX_CHUNKS 1024

#define pvPortMalloc malloc
#define pvPortFree free

struct track_entry {
  uint32_t album;
  uint32_t artist;
  uint32_t track_no;
  int32_t date;
  uint32_t disc_no;
  uint32_t length;
  char title[64];
  char path[META_PATH_LEN];
};

// struct index_entry {
//   uint32_t id;
//   uint64_t hash;
// };

// struct index_context {
//   uint32_t count;
//   uint32_t chunks;
//   uint32_t ram_max;
//   struct index_entry *entries;
// //  struct index_entry entries[DB_INDEX_RAM_COUNT];
//   char cache_file_name[DB_CACHE_FILE_NAME_LEN];
// };
// 
// struct track_index_entry {
//   uint32_t id;
//   uint64_t hash;
// };

// struct track_index_entry track_index[1024];

uint64_t database_hash(char *input) {
  uint64_t hash = 0;
  char temp[8];
  int i;
  strncpy(temp, input, 8);
  for(i=0;i<8;i++) {
    if((temp[i] >= 'a') && (temp[i] <= 'z')) {
      temp[i] -= ('a' - 'A');
    }
  }
/*  if(strncmp(temp, "THE ", 4) == 0) {
    return database_hash(&input[4]);
  }*/
  for(i=0;i<8;i++) {
    hash += ((uint64_t)temp[i] << (8 * (7-i)));
  }
  return hash;
}

// void db_index_sort_chunk(struct index_entry *idxe, int count) {
//   int i;
//   int changed = 1;
//   struct index_entry temp;
//   while(changed) {
//     changed = 0;
//     for(i=0;i<count-1;i++) {
//       if(idxe[i].hash > idxe[i+1].hash) {
//         temp.hash = idxe[i].hash;
//         temp.id = idxe[i].id;
//         idxe[i].hash = idxe[i+1].hash;
//         idxe[i].id = idxe[i+1].id;
//         idxe[i+1].hash = temp.hash;
//         idxe[i+1].id = temp.id;
//         changed = 1;
//       }
//     }
//   }
// }

// int db_index_init(struct index_context *idx, const char *cachefile) {
//   idx->count = 0;
//   idx->chunks = 0;
//   idx->ram_max = DB_INDEX_RAM_COUNT;
//   idx->entries = (struct index_entry *)pvPortMalloc(sizeof(struct index_entry) * DB_INDEX_RAM_COUNT);
//   if(strlen(cachefile) >= sizeof(idx->cache_file_name)) {
//     // cache file name is too long
//     return -1;
//   }
//   strncpy(idx->cache_file_name, cachefile, sizeof(idx->cache_file_name));
//   
//   return 0;
// }
  
// int db_index_append(struct index_context *idx, uint32_t id, uint64_t hash) {
//   int fd;
//   if(idx->count == idx->ram_max) {
//     // the in-memory list is full, have to write it to the cache file
//     // first sort it
//     db_index_sort_chunk(idx->entries, idx->count);
//     // now write the sorted list to the cache file
//     fd = open(idx->cache_file_name, O_WRONLY | O_CREAT, 0777);
//     lseek(fd, sizeof(struct index_entry) * idx->ram_max * idx->chunks, SEEK_SET);
//     write(fd, idx->entries, sizeof(struct index_entry) * idx->count);
//     close(fd);
//     idx->count = 0;
//     idx->chunks++;
//   }
//   idx->entries[idx->count].hash = hash;
//   idx->entries[idx->count++].id = id;
//   return 0;
// }

// int db_index_store(struct index_context *idx, FILE *fw) {
//   struct index_entry temp;
//   struct index_entry *chunk_heads;
//   int *chunk_counts;
//   int track_count;
//   int sorted_tracks;
//   int fr;
//   int i;
//   uint32_t min_i;
//   uint64_t min_h;
//   
//   // make sure the in-memory chunk is sorted
//   db_index_sort_chunk(idx->entries, idx->count);
//   // write it to the cache file
//   // open the cache file
//   fr = open(idx->cache_file_name, O_WRONLY | O_CREAT, 0777);
//   lseek(fr, (idx->ram_max * sizeof(struct index_entry) * idx->chunks), SEEK_SET);
//   write(fr, idx->entries, sizeof(struct index_entry) * idx->count);
// 
//   // pad the file out with maximum hash entries so they'll never get read
//   temp.id = UINT32_MAX;
//   temp.hash = UINT64_MAX;
//   for(i=idx->count;i<idx->ram_max;i++) {
//     write(fr, &temp, sizeof(struct index_entry));
//   }
//   idx->chunks++;
//   close(fr);
//   
//   pvPortFree(idx->entries);
//   
//   fr = open(idx->cache_file_name, O_RDONLY, 0777);
//   if(idx->chunks > MAX_CHUNKS) {
//     return -1;
//   }
//   chunk_heads = (struct index_entry *)pvPortMalloc(sizeof(struct index_entry) * idx->chunks);
//   chunk_counts = (int *)pvPortMalloc(sizeof(int) * idx->chunks);
//   for(i=0;i<idx->chunks;i++) {
//     chunk_counts[i] = idx->ram_max;
//     lseek(fr, i * sizeof(struct index_entry) * idx->ram_max, SEEK_SET);
//     read(fr, &chunk_heads[i], sizeof(struct index_entry));
//   }
//   
//   track_count = idx->count + (idx->chunks * idx->ram_max);
//   sorted_tracks = 0;
//   while(sorted_tracks < track_count) {
//     min_i = 0;
//     min_h = chunk_heads[0].hash;
//     for(i=1;i<idx->chunks;i++) {
//       if(chunk_counts[i] > 0) {
//         if(chunk_heads[i].hash < min_h) {
//           min_i = i;
//         }
//       }
//     }
//     fwrite(&chunk_heads[min_i].id, sizeof(uint32_t), 1, fw);
//     chunk_counts[min_i]--;
//     lseek(fr, (min_i * idx->ram_max + (idx->ram_max - chunk_counts[min_i]))  * sizeof(struct index_entry), SEEK_SET);
//     read(fr, &chunk_heads[min_i], sizeof(struct index_entry));
//     sorted_tracks++;
//   }
//   
//   close(fr);
//   
//   pvPortFree(chunk_heads);
//   pvPortFree(chunk_counts);
//   return 0;
// }

void drop_folder(char *filename) {
  int i;
  for(i=strlen(filename)-2;i>=0;i--) {
    if(filename[i] == '/') {
      filename[i+1] = 0;
      break;
    }
  }
}

void directory_part(char *filename) {
  int i;
  for(i=strlen(filename)-1;i>=0;i--) {
    if(filename[i] == '/') {
      filename[i+1] = 0;
      break;
    }
  }
}

int store_tracks(char *root_path) {
  struct meta meta_info;
  struct track_entry track;
  struct stat stat_buf;
  //struct index_context idx;
  FILE *fr;
  int fw;
  DIR *dr;
  struct dirent *de;
  int entry_no[MAX_NESTED_DIRS];
  char filename[256];
  uint32_t entry_count=0;
  int i=0;
  int depth=0;
  uint32_t idx;
  struct db_context c;
  
  btree_init("/home/nathan/track_cache", 8, &c);
  dr = opendir(root_path);
  fw = open("tracks.db", O_WRONLY | O_CREAT, 0777);
  lseek(fw, sizeof(uint32_t), SEEK_SET);
  
  for(i=0;i<MAX_NESTED_DIRS;i++) {
    entry_no[i] = 0;
  }
  
  strncpy(filename, root_path, sizeof(filename));
  
  //db_index_init(&idx, "/home/nathan/cache.idx");
  while(1) {
    if((de = readdir(dr))) {
      entry_no[depth]++;
      if(de->d_name[0] != '.') {
        strncpy(&filename[strlen(filename)], de->d_name, sizeof(filename) - strlen(filename));
        stat(filename, &stat_buf);
        if(S_ISDIR(stat_buf.st_mode)) {
          closedir(dr);
          filename[strlen(filename)+1] = 0;
          filename[strlen(filename)] = '/';
          
          dr = opendir(filename);
          depth++;
          entry_no[depth] = 0;
        } else if(strlen(de->d_name) > 3) {
          if(strncmp(".ogg", &de->d_name[strlen(de->d_name)-4], 4) ==  0) {
            fr = fopen(filename, "rb");

            read_standard_tags(fr, &meta_info);

            strncpy(track.title, meta_info.title, 64);
            track.album = 0;
            track.artist = 0;
            track.track_no = meta_info.track;
            track.date = meta_info.date;
            track.disc_no = meta_info.disc;
            strncpy(track.path, filename, META_PATH_LEN);
            fseek(fr, 0, SEEK_SET);
            track.length = ogg_track_length_millis(fr);
//             printf("name %s (%d) %s\r\n", track.path, (int)strlen(track.path), track.title);
//             printf("album %d\r\n", track.album);
//             printf("artist %d\r\n", track.artist);
//             printf("track_no %d\r\n", track.track_no);
//             printf("length %d\r\n", track.length);
//             printf("date %d\r\n", track.date);
//             printf("disc_no %d\r\n", track.disc_no);
            write(fw, &track, sizeof(track));
//             printf("Index entry [%d] => %lu\r\n", i, database_hash(track.title));
            //db_index_append(&idx, entry_count, database_hash(track.title));
//             printf("Entry %d\r\n", entry_count);
            btree_insert(entry_count, database_hash(track.title), c.head, 0, &c);
//             if(entry_count == 214) {
//               
//   FILE *fff = fopen("wibble214.html", "w");
//   fprintf(fff,"<p>%s</p>\r\n", track.title);
//   dbgoutput(fff, 214, c.head);
//   fclose(fff);
//             } else if(entry_count == 215) {
//               
//   FILE *fff = fopen("wibble215.html", "w");
//   fprintf(fff, "<p>%s</p>\r\n", track.title);
//   dbgoutput(fff, 215, c.head);
//   fclose(fff);
//               exit(1);
//             }
//             if(btree_check(&c)) {
//               printf("Error at %d\r\n", entry_count);
//               exit(1);
//             }
            fclose(fr);
            entry_count++;
          }
          directory_part(filename);
//           printf("5 %s\r\n", filename);
        } else {
          directory_part(filename);
//           printf("5 %s\r\n", filename);
        }
      }
    } else {
      // reached the end of the directory
      if(depth > 0) {
        closedir(dr);
        depth --;
        drop_folder(filename);
//         printf("%s\r\n", filename);
        dr = opendir(filename);
        for(i=0;i<entry_no[depth];i++) {
          de = readdir(dr);
        }
      } else {
        break;
      }
    }
  }
  closedir(dr);
  // now write the index at the end of the track list
  //db_index_store(&idx, fw);
  
//   if(btree_validate(&c)) {
//     exit(-1);
//   }
  
  btree_rewind(&c);
  while(btree_walk(&idx, &c)) {
    write(fw, &idx, 4);
  }
  /*
  dump_leaves(&c);
  FILE *fff = fopen("wibble.html", "w");
  dbgoutput(fff, 0, c.head);
  fclose(fff);*/
//   Node n;
//   for(i=0;i<4;i++) {
//     cache_get_node(i, &n);
//     printf("Node %d, parent: %d, isleaf: %d, pointers: [", i, n.parent, n.isleaf);
//     for(idx=0;idx<n.pointers_len;idx++) {
//       printf("%d, ", n.pointers[idx]);
//     }
// //     printf("]\r\n");
//   }
  
  lseek(fw, 0, SEEK_SET);
  write(fw, &entry_count, sizeof(entry_count));
  
  printf("Stored %d tracks\r\n", entry_count);
  
  close(fw);
  return 0;
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf("select directory\r\n");
    return -1;
  }
  store_tracks(argv[1]);
  
  return 0;
}
