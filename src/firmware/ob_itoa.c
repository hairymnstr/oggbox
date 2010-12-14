char *ob_itoa(int value, char *str, int base) {
  register int i=0, j;
  char temp;

  if(value == 0) {
    str[0] = '0';
    str[1] = 0;
    return str;
  }

  while(value > 0) {
    str[i++] = value % base;
    value = value / base;
  }
  i--;
  for(j=i;j>(i/2);j--) {
    temp = str[i-j];
    if(str[j] < 10) {
      str[i-j] = str[j] + '0';
    } else {
      str[i-j] = str[j] + 'a' - 10;
    }
    if(temp < 10) {
      str[j] = temp + '0';
    } else {
      str[j] = temp + 'a' - 10;
    }
  }
  if(!(i % 2)) {
    /* if it's odd number of chars so max index is even */
    /* need to do middle one separately */
    if(str[i/2] < 10) {
      str[i/2] += '0';
    } else {
      str[i/2] += 'a' - 10;
    }
  }
  str[++i] = 0;

  return str;
}
/*
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
*/
