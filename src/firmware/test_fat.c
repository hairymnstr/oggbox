#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "fat.h"
#include "block.h"
#include "block_pc.h"
#include "mbr.h"

/**************************************************************
 * Filesystem image structure:
 * 
 * root /+-> ROFILE.TXT
 *       +-> DIR1
 *       +-> NORMAL.TXT
 *************************************************************/

/**************************************************************
 * Error codes to test for on fat_open();
 * 
 * [EACCES] - write on a read only file
 * [EISDIR] - write access to a directory
 * [ENAMETOOLONG] - file path is too long
 * [ENFILE] - too many files are open
 * [ENOENT] - no file with that name or empty filename
 * [ENOSPC] - write to a full volume
 * [ENOTDIR] - part of subpath is not a directory but a file
 * [EROFS] - write access to file on read-only filesystem
 * [EINVAL] - mode is not valid
 **************************************************************/
int test_open(int p) {
  int i;
  int v;
  const char *desc[] = {"Test O_WRONLY on a read only file.",
                        "Test O_RDWR on a read only file.",
                        "Test O_RDONLY on a read only file.",
                        "Test O_WRONLY on a directory.",
                        "Test O_RDWR on a directory.",
                        "Test O_RDONLY on a directory.",
                        "Test O_WRONLY on a missing file.",
                        "Test O_RDWR on a missing file.",
                        "Test O_RDONLY on a missing file.",
                        "Test O_WRONLY on a path with file as non terminal member.",
                        "Test O_RDWR on a path with file as non terminal member.",
                        "Test O_RDONLY on a path with a file as non terminal member.",
  };
  const char *filename[] = {"/ROFILE.TXT",
                            "/ROFILE.TXT",
                            "/ROFILE.TXT",
                            "/DIR1",
                            "/DIR1",
                            "/DIR1",
                            "/MISSING.TXT",
                            "/MISSING.TXT",
                            "/MISSING.TXT",
                            "/ROFILE.TXT/NONE.TXT",
                            "/ROFILE.TXT/NONE.TXT",
                            "/ROFILE.TXT/NONE.TXT",
  };
  const int flags[] = {O_WRONLY,
                       O_RDWR,
                       O_RDONLY,
                       O_WRONLY,
                       O_RDWR,
                       O_RDONLY,
                       O_WRONLY,
                       O_RDWR,
                       O_RDONLY,
                       O_WRONLY,
                       O_RDWR,
                       O_RDONLY,
  };
  const int result[] = {EACCES,
                        EACCES,
                        0,
                        EISDIR,
                        EISDIR,
                        0,
                        ENOENT,
                        ENOENT,
                        ENOENT,
                        ENOTDIR,
                        ENOTDIR,
                        ENOTDIR,
  };
  const int cases = 12;
  
  int rerrno;
  
  for(i=0;i<cases;i++) {
    printf("[%4d] Testing %s", p++, desc[i]);
    v = fat_open(filename[i], flags[i], 0, &rerrno);
    if(rerrno == result[i]) {
      printf("  [ ok ]\n");
    } else {
      printf("  [fail]\n  expected (%d) %s\n  got (%d) %s\n", result[i], strerror(result[i]), rerrno, strerror(-rerrno));
    }
    if(v > -1) {
      fat_close(v, &rerrno);
      if(rerrno != 0) {
        printf("fat_close returned %d (%s)\n", rerrno, strerror(rerrno));
      }
      
    }
  }
  return p;
}

int main(int argc, char *argv[]) {
  int p = 0;
  int rerrno = 0;
  FILE *fp;
  int len;
  uint8_t *d;
//   int v;
  printf("Running FAT tests...\n\n");
  printf("[%4d] start block device emulation...", p++);
  printf("   %d\n", block_init());
  
  printf("[%4d] mount filesystem, FAT32", p++);
  printf("   %d\n", fat_mount(0, PART_TYPE_FAT32));

//   p = test_open(p);

  int fd;
  
//   printf("Open\n");
//   fd = fat_open("/newfile.txt", O_WRONLY | O_CREAT, 0777, &rerrno);
//   printf("fd = %d, errno=%d (%s)\n", fd, rerrno, strerror(rerrno));
//   if(fd > -1) {
//     printf("Write\n");
//     fat_write(fd, "Hello World\n", 12, &rerrno);
//     printf("errno=%d (%s)\n", rerrno, strerror(rerrno));
//     printf("Close\n");
//     fat_close(fd, &rerrno);
//     printf("errno=%d (%s)\n", rerrno, strerror(rerrno));
//   }
  
  printf("Open\n");
  fd = fat_open("/newfile.png", O_WRONLY | O_CREAT, 0777, &rerrno);
  printf("fd = %d, errno=%d (%s)\n", fd, rerrno, strerror(rerrno));
  if(fd > -1) {
    fp = fopen("/home/nathan/gowrong_draft1.png", "rb");
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    d = malloc(len);
    fseek(fp, 0, SEEK_SET);
    fread(d, 1, len, fp);
    fclose(fp);
    printf("Write PNG\n");
    fat_write(fd, d, len, &rerrno);
    printf("errno=%d (%s)\n", rerrno, strerror(rerrno));
    printf("Close\n");
    fat_close(fd, &rerrno);
    printf("errno=%d (%s)\n", rerrno, strerror(rerrno));
  }
  
  block_pc_snapshot_all("writenfs.img");
  exit(0);
}
