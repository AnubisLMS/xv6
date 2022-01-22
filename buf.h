#ifndef XV6_BUF_H
#define XV6_BUF_H

#include "fs.h"

struct buf {
  int flags;
  unsigned int dev;
  unsigned int blockno;
  struct buf* prev; // LRU cache list
  struct buf* next;
  struct buf* qnext; // disk queue
  unsigned char data[BSIZE];
};
#define B_BUSY 0x1  // buffer is locked by some process
#define B_VALID 0x2 // buffer has been read from disk
#define B_DIRTY 0x4 // buffer needs to be written to disk

#endif
