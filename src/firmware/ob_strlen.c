int ob_strlen(const char *str) {
  int i=0;
  const char *c = str;

  while(*c++ != 0) {
    i++;
  }
  return i;
}

