#ifndef UNICODE_TABLE_H
#define UNICODE_TABLE_H 1

struct unicode_item {
  int value;
  char as_ascii[4];
};

extern const int unicode_table_len;
extern const struct unicode_item unicode_table[];

#endif /* ifndef UNICODE_TABLE_H */
