#include <stdio.h>
#include <string.h>
#include "unicode_table.h"
#include "unicode.h"

const char *null_str = "";

int utf8_to_value(unsigned char *str, int len, int *val) {
  int bytelen;
  int v;
  int i;
  if(str[0] < 0x80) {
    // the ascii is identical to utf8
    *val = str[0];
    return 1;
  }
  if(str[0] < 0xC0) {
    return -1;          // first character of a UTF8 sequence must have at least two high bit ones
  }
  if((str[0] & 0xFC) == 0xFC) {
    // 6 byte sequence
    if(len < 6) {
      return -1;
    }
    bytelen = 6;
    v = (str[0] & 1);
  } else if((str[0] & 0xF8) == 0xF8) {
    // 5 byte sequence
    if(len < 5) {
      return -1;
    }
    bytelen = 5;
    v = (str[0] & 3);
  } else if((str[0] & 0xF0) == 0xF0) {
    // 4 byte sequence
    if(len < 4) {
      return -1;
    }
    bytelen = 4;
    v = (str[0] & 7);
  } else if((str[0] & 0xE0) == 0xE0) {
    // 3 byte sequence
    if(len < 3) {
      return -1;
    }
    bytelen = 3;
    v = (str[0] & 0xF);
  } else if((str[0] & 0xC0) == 0xC0) {
    // 2 byte sequence
    if(len < 2) {
      return -1;
    }
    bytelen = 2;
    v = (str[0] & 0x1F);
  }
  
  for(i=1;i<bytelen;i++) {
    if((str[i] & 0xC0) != 0x80) {
      return -1;                // this isn't a UTF8 continuation byte
    }
    v = (v << 6) | (str[i] & 0x3F);
  }
  
  *val = v;
  return bytelen;
}

const char *utf8_to_ascii(int val) {
  int i;
  for(i=0;i<unicode_table_len;i++) {
    if(unicode_table[i].value == val) {
      return unicode_table[i].as_ascii;
    }
  }
  return null_str;
}

char *utf8_strncpy(char *s1, char *s2, int n) {
  char *c = s1;
  int count = 0;
  int v;
  const char *r;
  int l;
  while(count < n) {
    if((*s2) == 0) {
      *c = 0;
      break;
    } else if((*s2) > 0 ) {
      *c++ = *s2++;
      count++;
    } else {
      l = utf8_to_value((unsigned char *)s2, strlen(s2), &v);
      if(l > -1) {
        // lookup value in conversion table
        r = utf8_to_ascii(v);
        if((count + strlen(r)) > (n-1)) {
          *c = 0;
          break;
        } else {
          for(v=0;v<strlen(r);v++) {
            *c++ = r[v];
            count++;
          }
        }
        s2 += l;
      } else {
        // bad UTF-8 sequence just dump this character
        s2++;
      }
    }      
  }
  return s1;
}
