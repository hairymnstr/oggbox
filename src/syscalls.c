#include "config.h"
#include "fat.h"
#include "nd_usart.h"
#include <libopencm3/stm32/f1/rtc.h>
#include <libopencm3/stm32/usart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

const char *seek_str[] = {
  "SEEK_SET",
  "SEEK_CUR",
  "SEEK_END",
};

/**
 *  write_std_out - writes len bytes from char *buffer to the "standard out"
 *                  UART as specified in config.h
 */
int write_std_out(const void *buffer, int len) {
  int c = len;
  char *cp = (char *)buffer;

  while(c--) {
    usart_send_blocking(STD_OUT_UART, *cp++);
  }

  return len;
}

/**
 *  write_std_err - writes errors to the UART specified in config.h
 */
int write_std_err(const void *buffer, int len) {
  int c = len;
  char *cp = (char *)buffer;
  
  while(c--) {
    usart_send_blocking(STD_ERR_UART, *cp++);
  }

  return len;
}

/**
 *  function definitions for newlib's low-level calls
 */
void _exit (int n) {
  while(1) {
    n = n;
  }
}

int _stat(char *file, struct stat *st) {
  int i;
  int rerr;
  i = fat_open(file, O_RDONLY, 0777, &rerr);
  if(i < 1)
    return -1;
  fat_fstat(i, st, &rerr);
  fat_close(i, &rerr);
  return (0);
}

int _fstat (int fd, struct stat * st) {
  int rerr;
  write_std_out("_fstat", 6);
  usart_dec_u32(fd);
  write_std_out("\r\n", 2);
  if (fd == STDOUT_FILENO) {
    memset(st, 0, sizeof (* st));
    st->st_mode = S_IFCHR;
    st->st_blksize = 1024;
    return 0;
  } else if (fd >= FIRST_DISC_FILENO) {
    fat_fstat(fd - FIRST_DISC_FILENO, st, &rerr);
    return 0;
  } else {
    write_std_err("Error: _fstat\r\n", 15);
    return(-1);
  }
}

register char *stack_ptr asm ("sp");
caddr_t _sbrk_r(void *reent __attribute__((__unused__)), size_t incr) {
  extern char end asm ("end");  // Defined by the linker
  static char *heap_end;
  char *prev_heap_end;

  write_std_out("_sbrk\r\n", 6);

  if( heap_end == NULL )
    heap_end = &end;
  prev_heap_end = heap_end;

  if(( heap_end + incr ) > stack_ptr ) {
    write_std_out("heap<>stack collision\n", 22);
    //exit(1);
    return (caddr_t) -1;
  }

  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

int _isatty(int fd) {
  if((fd == STDOUT_FILENO) || (fd == STDIN_FILENO) || (fd == STDERR_FILENO))
    return 1;
  return 0;
}

int _lseek(int fd, int ptr, int dir) {
  int rerr, v;
//   iprintf("_lseek %d, %d, %d\r\n", fd, ptr, dir);
  if(fd < FIRST_DISC_FILENO) {
    // tried to seek on stdin/out/err
    return ptr-1;
  }
  v = fat_lseek(fd - FIRST_DISC_FILENO, ptr, dir, &rerr);
  if(v == ptr-1) {
    iprintf("Lseek(%d, %d, %s) errno=%d\r\n", fd, ptr, seek_str[dir], rerr);
  }
  return v;
}

int _open_r(struct _reent *ptr, const char *name, int flags, int mode) {
  int i;
  int rerrno;
  i = fat_open(name, flags, mode, &rerrno);

  if(i<0) {
    ptr->_errno = rerrno;
    i = -1;
  } else {
    i += FIRST_DISC_FILENO;     /* otherwise we'd be returning STD* filenos */
  }
  return i;
}

int _close(int fd) {
  int rerr;
  fat_close(fd - FIRST_DISC_FILENO, &rerr);
  return(0);
}

int _write(int fd, const void *data, unsigned int count) {
  if (fd == STDOUT_FILENO)  {
    count = write_std_out(data, count);
  } else if (fd == STDERR_FILENO) {
    count = write_std_err(data, count);
  } else {
    write_std_err("Error: _write\r\n", 15);
    count = -1;
  }
  return(count);
}

int _read(int fd, void *buffer, unsigned int count) {
  int rerr;
  //write_std_out("_read\n", 6);
  if (fd == STDOUT_FILENO) {
    return -1;
  } else if (fd == STDERR_FILENO) {
    return -1;
  } else if (fd == STDIN_FILENO) {
    return(-1);   // not implemented
  } else if (fd >= FIRST_DISC_FILENO) {
    return fat_read(fd - FIRST_DISC_FILENO, buffer, count, &rerr);
  }
  return -1;
} 

int _gettimeofday(struct timeval *tp, void *tzp __attribute__((__unused__))) {
  tp->tv_sec = rtc_get_counter_val();
  tp->tv_usec = 0;
  return 0;
}
