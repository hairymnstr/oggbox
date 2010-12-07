char *ob_strtuc(char *str) {
 char *c = str;

 while((*c) != 0) {
    if(((*c) >= 'a') && ((*c) <= 'z')) {
      (*c) -= ('a' - 'A');
    }
    c++;
  }
  return str;
}

char *ob_strtlc(char * str) {
  char *c = str;
  while((*c) != 0) {
    if(((*c) >= 'A') && ((*c) <= 'Z')) {
      (*c) -= ('A' - 'a');
    }
    c++;
  }
  return str;
}

