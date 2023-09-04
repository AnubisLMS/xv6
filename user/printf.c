#include "user.h"

static void putc(int fd, char c) {
  write(fd, &c, 1);
}

static void printint(int fd, int xx, int base, int sgn) {
  static char digits[] = "0123456789ABCDEF";
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0) {
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  while(--i >= 0)
    putc(fd, buf[i]);
}

void vfprintf(int fd, char* fmt, unsigned* ap) {
  char* s;
  int c, i, state;

  state = 0;
  for(i = 0; fmt[i]; i++) {
    c = fmt[i] & 0xff;
    if(state == 0) {
      if(c == '%') {
        state = '%';
      } else {
        putc(fd, c);
      }
    } else if(state == '%') {
      if(c == 'd') {
        printint(fd, *ap, 10, 1);
        ap++;
      } else if(c == 'x' || c == 'p') {
        printint(fd, *ap, 16, 0);
        ap++;
      } else if(c == 's') {
        s = (char*) *ap;
        ap++;
        if(s == 0)
          s = "(null)";
        while(*s != 0) {
          putc(fd, *s);
          s++;
        }
      } else if(c == 'c') {
        putc(fd, *ap);
        ap++;
      } else if(c == '%') {
        putc(fd, c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c);
      }
      state = 0;
    }
  }
}

void printf(char* fmt, ...) {
  vfprintf(1, fmt, (unsigned*) (void*) &fmt + 1);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void fprintf(int fd, char* fmt, ...) {
  vfprintf(1, fmt, (unsigned*) (void*) &fmt + 1);
}
