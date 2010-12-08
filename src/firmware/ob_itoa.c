char *ob_itoa(int value, char *str, int base) {
  int cb = base, bc=1;
  int i;

  while(value > cb) {
    cb *= base;
    bc++;
  }
  cb = base;
  for(i=bc-1;i>=0;i--) {
    str[i] = (value % base);
    value = value / cb;
    cb *= base;
  }
  for(i=0;i<bc;i++) {
    if(str[i] < 10) {
      str[i] += '0';
    } else {
      str[i] += 'a' - 10;
    }
  }
  str[i] = 0;
  return str; 
}

