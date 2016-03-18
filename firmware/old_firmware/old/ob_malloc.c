#include "ob_malloc.h"
#include <stdio.h>

/**
 *  ob_malloc - custom OggBox malloc function, uses a cluster allocation table
 *              to make best use of available space and to reliably free()
 **/

unsigned char alloc_table[1280];

int ob_malloc_init(int static_end, int stack_top) {
  register int i;

  if(stack_top < static_end) {
    return 1;
  }

  if(static_end % 4)
    static_end = (static_end / 4) + 1;
  else
    static_end /= 4;

  if(stack_top % 4)
    stack_top = (stack_top / 4) + 1;
  else
    stack_top = (stack_top / 4);

  for(i=0;i<static_end;i++) {
    alloc_table[i/4] |= (3 << ((i % 4) * 2));
  }
  for(i=static_end;i<stack_top;i++) {
    alloc_table[i/4] &= ~(3 << ((i % 4) * 2));
  }
  for(i=stack_top;i<5120;i++) {
    alloc_table[i/4] |= (3 << ((i % 4) * 2));
  }
  return 0;
}

void *ob_malloc(int len) {
  unsigned char code;
  int ptr, sptr;
  int found_len;

  /* can only allocate in blocks of 4 bytes */
  ptr = 0;
  while(ptr < 5120) {
    do {
      code = (alloc_table[ptr/4] >> ((ptr % 4) * 2)) & 3;
      ptr++;
    } while(code != 0);

    found_len = 4;
    sptr = ptr - 1;
    while(found_len < len) {
      code = (alloc_table[ptr/4] >> ((ptr % 4) * 2)) & 3;
      ptr++;
      if(code == 0) {
        found_len += 4;
      } else {
        break;
      }
    }
    if(found_len >= len) {
      /* found a chunk of space suitable */
      ptr = sptr;
      for(ptr=sptr;ptr<sptr+(found_len/4)-1;ptr++) {
        alloc_table[ptr/4] |= (1 << ((ptr % 4) * 2));
      }
      alloc_table[(sptr+(found_len/4)-1)/4] |= 
                  (2 << (((sptr+(found_len/4)-1) % 4) *2));
      /* return a void * pointer to the start of the allocated chunk */
      return (void *)(sptr * 4);
    }
  }
  return (void *)0;   /* failed to find enough memory, return null ptr */
}

void ob_free(void *ptr) {
  int off = (int)ptr;

  unsigned char code;

  off /= 4;

  if(!((((alloc_table[off/4] >> ((off % 4) * 2)) & 3) == 1) ||
       (((alloc_table[off/4] >> ((off % 4) * 2)) & 3) == 2))) {
    /* tried to free some memory that is not allocated through ob_malloc */
    ob_segfault("Free memory that's not allocated");
  }

  /* follow a chain of allocation entries with state 1 until an end segment
     state (2) is found */
  do {
    code = (alloc_table[off/4] >> ((off % 4) * 2)) & 3;
    alloc_table[off/4] &= ~(3 << ((off % 4) * 2));
    off++;
  } while(code == 1);

  if(!(code == 2))
    ob_segfault("memory block not ended");

  return;
}

void ob_segfault(char *msg) {
  printf("SEGFAULT: %s\n", msg);
  while(1) {;}
}

