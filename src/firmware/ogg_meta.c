#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "meta.h"
#include "meta_db.h"

uint32_t tag_split(char *buf) {
  int i;
  for(i=0;i<256;i++) {
    if(buf[i] == '=') {
      return i;
    }
    if((buf[i] >= 'a') && (buf[i] <= 'z')) {
      buf[i] -= 0x20;
    }
  }
  return 256;
}

char *split_ext(char *path) {
  int i;

  i = strlen(path);
  while((path[i] != '.') && (path[i] != '/') && (i > 0)) {
    i--;
  }
  if(i == 0) {
    return NULL;
  }
  if(path[i] == '/') {
    return NULL;
  }
  return &path[i+1];
}

int find_meta_start(FILE *fr) {
  char buf[4];
  char page_segments;
  int page_len;
  int i;

  fread(buf, 1, 4, fr);
  
  if(strncmp(buf, "OggS", 4) != 0) {
    return -1;
  }

  fseek(fr, 26, SEEK_SET);

  fread(&page_segments, 1, 1, fr);

  page_len = 0;

  for(i=0;i<page_segments;i++) {
    fread(buf, 1, 1, fr);
    page_len += buf[0];
  }
  
  fseek(fr, 27 + page_segments + page_len, SEEK_SET);

  fread(buf, 1, 4, fr);
  
  if(strncmp(buf, "OggS", 4) != 0) {
    return -2;
  }

  return 27 + page_segments + page_len;
}

int read_standard_tags(FILE *fr, struct meta *out) {
  uint32_t len, tagcount;
  char buf[256];
  uint32_t mark;
  int i;
  uint8_t page_segments;
  int meta_start;

  // terminate all strings at zero length to start with
  out->title[0] = 0;
  out->album[0] = 0;
  out->artist[0] = 0;

  meta_start = find_meta_start(fr);

  fseek(fr, meta_start, SEEK_SET);
  fread(buf, 1, 4, fr);
  if(strncmp(buf, "OggS", 4) != 0) {
    printf("oops.\n");
    return -1;
  }

  fseek(fr, meta_start + 26, SEEK_SET);
  fread(&page_segments, 1, 1, fr);

  fseek(fr, meta_start + 27 + page_segments, SEEK_SET);
  fread(buf, 1, 1, fr);
  if(buf[0] != 3) {
    printf("not a comment packet :O\n");
    return -1;
  }
  fseek(fr, 6, SEEK_CUR);

  fread(&len, 4, 1, fr);
  fseek(fr, len, SEEK_CUR);
  fread(&tagcount, 4, 1, fr);
  for(i=0;i<tagcount;i++) {
    fread(&len, 4, 1, fr);
    if(len < 256) {
      fread(buf, 1, len, fr);
      buf[len] = 0;
    } else {
      fread(buf, 1, 255, fr);
      buf[255] = 0;
      fseek(fr, len - 255, SEEK_CUR);
    }
    mark = tag_split(buf);

    if((strncmp(buf,"ARTIST", 6) == 0) && (mark == 6)) {
      strncpy(out->artist, &buf[mark+1], META_STR_LEN);
    } else if((strncmp(buf, "TITLE", 5) == 0) && (mark == 5)) {
      strncpy(out->title, &buf[mark+1], META_STR_LEN);
    } else if((strncmp(buf, "ALBUM", 5) == 0) && (mark == 5)) {
      strncpy(out->album, &buf[mark+1], META_STR_LEN);
    } else if((strncmp(buf, "TRACKNUMBER", 11) == 0) && (mark == 11)) {
      out->track = 0;
      sscanf(&buf[mark+1], "%u", &out->track);
    } else if((strncmp(buf, "DATE", 4) == 0) && (mark == 4)) {
      out->date = 0;
      sscanf(&buf[mark+1], "%d", &out->date);
    }
  }
  return 0;
}

int parse_dirs(char *path, struct db_context *context) {
  DIR *dr;
  struct dirent *de;
  int plen = strlen(path);
  struct stat st;
  char *ext;
  FILE *fr;
  struct meta m;
  char *temp;

  if(path[plen-1] != '/') {
    fprintf(stderr, "E1: Not a / terminated path.\r\n");
    return -1;
  }
  
  dr = opendir(path);

  while((de = readdir(dr))) {
    if(de->d_name[0] != '.') {
      if(strlen(de->d_name) + plen > (META_MAX_PATH-1)) {
        fprintf(stderr, "E2: Can't handle paths that long\r\n");
        return -2;
      }
      strcpy(&path[plen], de->d_name);
      if(stat(path, &st)) {
        fprintf(stderr, "E3: Stat failed on %s, %d\r\n", path, errno);
        return -3;
      }
      if(st.st_mode & S_IFDIR) {
        path[strlen(path) + 1] = 0;
        path[strlen(path)] = '/';
        parse_dirs(path, context);
      } else {
        if((ext = split_ext(path))) {
          if(strncmp(ext, "ogg", 3) == 0) {
            // save the meta data from the file at path here!!
            if((fr = fopen(path, "rb"))) {
              read_standard_tags(fr, &m);
              printf("%s\r\n", path);
              printf("Artist:   %s\r\n", m.artist);
              printf("Album:    %s\r\n", m.album);
              printf("Title:    %s\r\n", m.title);
              printf("Date:     %d\r\n", m.date);
              printf("Track no: %u\r\n", m.track);
              fclose(fr);
              temp = (char *)malloc(sizeof(char) * META_STR_LEN);
              strcpy(temp, m.artist);
              meta_db_insert(temp, meta_db_string_hash(temp), context->head, -1, context);
            }
            
          }
        }
      }
    }
  }

  path[plen] = 0;
  closedir(dr);
  return 0;
}

void dbgoutput(FILE *fw, uint64_t item, Node *head) {
  char job_names[5000][50];
  Node *job_pointers[5000];
  int job_front = 0;
  int job_end = 1;
  int tableno = 1;
  int i;
  
  strcpy(job_names[0], "Root");
  job_pointers[0] = head;
  
  fprintf(fw, "Store %lu\n", item);
  while(job_front != job_end) {
    fprintf(fw, "<h2>%s</h2>\n", job_names[job_front]);
    fprintf(fw, "<h3>Parent: %p</h3>\n", job_pointers[job_front]->parent);
    fprintf(fw, "<table>\n");
    fprintf(fw, "<tr>");
    fprintf(fw, "<td></td>");
    for(i=0;i<METADB_NODE_SIZE;i++) {
      if(job_pointers[job_front]->keys_len > i)
        fprintf(fw, "<td>%lu</td><td></td>", job_pointers[job_front]->keys[i]);
      else
        fprintf(fw, "<td></td><td></td>");
    }
    fprintf(fw, "</tr>\n<tr>");
    
    for(i=0;i<METADB_NODE_SIZE+1;i++) {
      if(job_pointers[job_front]->pointers_len > i) {
        if(job_pointers[job_front]->isleaf) {
          fprintf(fw, "<td>%p</td><td></td>", job_pointers[job_front]->pointers[i]);
        } else {
          fprintf(fw, "<td>Table %d</td><td></td>", tableno);
          sprintf(job_names[job_end], "Table %d", tableno++);
          job_pointers[job_end++] = job_pointers[job_front]->pointers[i];
        }
      } else {
        fprintf(fw, "<td></td><td></td>");
      }
    }
    fprintf(fw, "</tr>\n<tr>");
    
    for(i=0;i<METADB_NODE_SIZE+1;i++) {
      if(job_pointers[job_front]->pointers_len > i) {
        if(job_pointers[job_front]->isleaf) {
          fprintf(fw, "<td>%s</td><td></td>", ((char *)(job_pointers[job_front]->pointers[i])));
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

int main(int argc, char *argv[]) {
  char path[META_MAX_PATH] = "/home/nd222/Music/";
  
  FILE *fw; //, *frandom;
  int i;
  uint64_t *item;
  struct db_context context;
  
  meta_db_init(&context);

  parse_dirs(path, &context);
  fw = fopen("debug.html", "w");

  fprintf(fw, "<html><head><title>DB Debug</title></head><body>");

//   frandom = fopen("/dev/urandom", "rb");
//   for(i=0;i<30;i++) {
//     item = (uint64_t *)malloc(sizeof(uint64_t));
//     *item = 0;
//     fread(item, 1, 1, frandom);

//     meta_db_insert(item, *item, context.head, -1, &context);

//     dbgoutput(fw, *item, context.head);
//   }
  dbgoutput(fw, 0, context.head);
  
  fprintf(fw, "Database size: %d\n", context.size);

  fprintf(fw, "</body></html>");
  fclose(fw);

//   exit(0);
  //FILE *fr;
  //struct ogg_meta meta;

  //fr = fopen("/home/nd222/Music/Daft Punk - Tron Legacy/03 - The Son Of Flynn.ogg", "rb");

  //int meta_start = find_meta_start(fr);
  //printf("%d\n", meta_start);

  //read_standard_tags(fr, meta_start, &meta);
  //fclose(fr);

  
  exit(0);
}
