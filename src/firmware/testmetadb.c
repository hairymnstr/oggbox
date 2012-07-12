#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "meta_db.h"

void dbgoutput(FILE *fw, uint64_t item, Node *head) {
  char job_names[50][50];
  Node *job_pointers[50];
  int job_front = 0;
  int job_end = 1;
  int tableno = 1;
  
  fprintf(fw, "Store %lu\n", item);
  while(job_front != job_end) {
    fprintf(fw, "<h2>%s</h2>\n", job_names[job_front]);
    fprintf(fw, "<h3>Parent: %p</h3>\n", job_pointers[job_front]->parent);
    fprintf(fw, "<table>\n");
    fprintf(fw, "<tr>");
    fprintf(fw, "<td></td>");
    for(i=0;i<METADB_NODE_SIZE;i++) {
      if(jobs_pointers[job_front]->keys_len > i)
        fprintf(fw, "<td>%lu</td><td></td>", job_pointers[job_front]->keys[i]);
      else
        fprintf(fw, "<td></td><td></td>");
    }
    fprintf(fw, "</tr>\n<tr>")
    
    for(i=0;i<METADB_NODE_SIZE+1;i++) {
      if(
}

int main(int argc, char *argv[]) {
  FILE *fw;
  int i;
  uint64_t *item;
  Node *head;
  
  head = (Node *)malloc(sizeof(Node));

  fw = fopen("debug.html", "w");

  fprintf(fw, "<html><head><title>DB Debug</title></head><body>");

  frandom = fopen("/dev/urandom", "rb");
  for(i=0;i<30;i++) {
    item = (uint64_t *)malloc(sizeof(uint64_t));
    fread(item, sizeof(uint64_t), 1, frandom);

    dbinsert(item, *item, &head, -1);

    dbgoutput(fw, *item, head);
  }
  dbgoutput(fw, 0, head);

  fprintf(fw, "</body></html>");
  fclose(fw);

  exit(0);
}
