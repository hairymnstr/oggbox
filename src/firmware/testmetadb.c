#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "meta_db.h"

void dbgoutput(FILE *fw, uint64_t item, Node *head) {
  char job_names[50][50];
  Node *job_pointers[50];
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
          fprintf(fw, "<td>%lu</td><td></td>", *((uint64_t *)(job_pointers[job_front]->pointers[i])));
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
  FILE *fw, *frandom;
  int i;
  uint64_t *item;
  struct db_context context;
  
  meta_db_init(&context);

  fw = fopen("debug.html", "w");

  fprintf(fw, "<html><head><title>DB Debug</title></head><body>");

  frandom = fopen("/dev/urandom", "rb");
  for(i=0;i<30;i++) {
    item = (uint64_t *)malloc(sizeof(uint64_t));
    *item = 0;
    fread(item, 1, 1, frandom);

    meta_db_insert(item, *item, context.head, -1, &context);

    dbgoutput(fw, *item, context.head);
  }
  dbgoutput(fw, 0, context.head);
  
  fprintf(fw, "Database size: %d\n", context.size);

  fprintf(fw, "</body></html>");
  fclose(fw);

  exit(0);
}
