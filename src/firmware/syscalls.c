#include "config.h"
#include "sdfat.h"
#include <libopencm3/stm32/usart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

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
  return (0);
}

int _fstat (int fd, struct stat * st) {
  write_std_out("_fstat", 6);
  write_std_out((char *)&fd, 4);
  write_std_out("\n", 1);
  if (fd == STDOUT_FILENO) {
    memset(st, 0, sizeof (* st));
    st->st_mode = S_IFCHR;
    st->st_blksize = 1024;
    return 0;
  } else if (fd >= FIRST_DISC_FILENO) {
    sdfat_stat(fd - FIRST_DISC_FILENO, st);
    return 0;
  } else {
    write_std_err("Error: _fstat\r\n", 15);
    return(-1);
  }
}

register char *stack_ptr asm ("sp");
caddr_t _sbrk_r(void *reent, size_t incr) {
  extern char end asm ("end"); // Defined by the linker
  static char *heap_end;
  char *prev_heap_end;

  write_std_out("_sbrk\n", 6);

  if( heap_end == NULL )
    heap_end = &end;
  prev_heap_end = heap_end;

  if(( heap_end + incr ) > stack_ptr ) {
    write_std_out("heap<>stack collision\n", 22);
    exit(1);
    return (caddr_t) -1;
  }

  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

int _isatty(int fd) {
  write_std_out("_isatty\n", 8);
  return(1);      // not implemented
}

int _lseek(int fd, int ptr, int dir) {
  if(fd < FIRST_DISC_FILENO) {
    // tried to seek on stdin/out/err
    return -1;
  }
  return sdfat_seek(fd - FIRST_DISC_FILENO, ptr, dir);
}

int _open_r(struct _reent *ptr, const char *name, int mode) {
  int i;
  i = sdfat_open(name, mode);

  if(i<0) {
    ptr->_errno = -i;
    i = -1;
  } else {
    i += FIRST_DISC_FILENO;     /* otherwise we'd be returning STD* filenos */
  }
  return i;
}

int _close(int fd) {
  sdfat_close(fd - FIRST_DISC_FILENO);
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
  //write_std_out("_read\n", 6);
  if (fd == STDOUT_FILENO) {
    return -1;
  } else if (fd == STDERR_FILENO) {
    return -1;
  } else if (fd == STDIN_FILENO) {
    return(-1);   // not implemented
  } else if (fd >= FIRST_DISC_FILENO) {
    return sdfat_read(fd - FIRST_DISC_FILENO, buffer, count);
  } else {
    return -1;
  }
} 

