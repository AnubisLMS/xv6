// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "defs.h"
#include "file.h"
#include "fs.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "spinlock.h"
#include "traps.h"
#include "x86.h"

static void consputc(int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void printint(int xx, int base, int sign) {
  static char digits[] = "0123456789abcdef";
  char buf[16];

  sign = sign && xx < 0;
  unsigned int x = sign ? -xx : xx;

  int i;
  for(i = 0; x; i++, x /= base)
    buf[i] = digits[x % base];

  if(sign)
    consputc('-');

  while(--i >= 0)
    consputc(buf[i]);
}

// Print to the console. only understands %d, %x, %p, %s.
void cprintf(char* fmt, ...) {
  int locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if(fmt == 0)
    panic("null fmt");

  unsigned int* argp = (unsigned int*) (&fmt + 1);
  for(int c, i = 0; (c = fmt[i] & 0xff); i++) {
    if(c != '%') {
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c) {
      case 'd':
        printint(*argp++, 10, 1);
        break;
      case 'x':
      case 'p':
        printint(*argp++, 16, 0);
        break;
      case 's': {
        char* s = (char*) *argp++;
        if(!s)
          s = "(null)";
        for(; *s; s++)
          consputc(*s);
      } break;
      case '%':
        consputc('%');
        break;
      default:
        // Print unknown % sequence to draw attention.
        consputc('%');
        consputc(c);
        break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void panic(char* s) {
  cli();
  cons.locking = 0;
  cprintf("cpu%d: panic: %s\n", cpu->id, s);

  unsigned int pcs[10];
  getcallerpcs(&s, pcs);
  cprintf("%p", pcs[0]);
  for(int i = 1; i < 10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;) {}
}

#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static unsigned short* crt = (unsigned short*) P2V(0xb8000); // CGA memory

static void cgaputc(int c) {
  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  int pos = inb(CRTPORT + 1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT + 1);

  if(c == '\n')
    pos += 80 - pos % 80;
  else if(c == BACKSPACE) {
    if(pos > 0)
      --pos;
  } else
    crt[pos++] = (c & 0xff) | 0x0700; // black on white

  if(pos < 0 || pos > 25 * 80)
    panic("pos under/overflow");

  if((pos / 80) >= 24) { // Scroll up.
    memmove(crt, crt + 80, sizeof(crt[0]) * 23 * 80);
    pos -= 80;
    memset(crt + pos, 0, sizeof(crt[0]) * (24 * 80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT + 1, pos >> 8);
  outb(CRTPORT, 15);
  outb(CRTPORT + 1, pos);
  crt[pos] = ' ' | 0x0700;
}

void consputc(int c) {
  if(panicked) {
    cli();
    for(;;) {}
  }

  if(c == BACKSPACE) {
    uartputc('\b');
    uartputc(' ');
    uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  unsigned int r; // Read index
  unsigned int w; // Write index
  unsigned int e; // Edit index
} input;

#define C(x) ((x) - '@') // Control-x

void consoleintr(int (*getc)(void)) {
  int doprocdump = 0;

  acquire(&cons.lock);
  for(int c; (c = getc()) >= 0;) {
    switch(c) {
      case C('P'):      // Process listing.
        doprocdump = 1; // procdump() locks cons.lock indirectly; invoke later
        break;
      case C('U'): // Kill line.
        while(input.e != input.w &&
            input.buf[(input.e - 1) % INPUT_BUF] != '\n') {
          input.e--;
          consputc(BACKSPACE);
        }
        break;
      case C('H'):
      case '\x7f': // Backspace
        if(input.e != input.w) {
          input.e--;
          consputc(BACKSPACE);
        }
        break;
      default:
        if(c != 0 && input.e - input.r < INPUT_BUF) {
          c = (c == '\r') ? '\n' : c;
          input.buf[input.e++ % INPUT_BUF] = c;
          consputc(c);
          if(c == '\n' || c == C('D') || input.e == input.r + INPUT_BUF) {
            input.w = input.e;
            wakeup(&input.r);
          }
        }
        break;
    }
  }
  release(&cons.lock);
  if(doprocdump)
    procdump(); // now call procdump() wo. cons.lock held
}

int consoleread(struct inode* ip, char* dst, int n) {
  unsigned int target = n;

  iunlock(ip);
  acquire(&cons.lock);
  while(n) {
    while(input.r == input.w) {
      if(proc->killed) {
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }

    int c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')) { // EOF
      if(n < target) {
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int consolewrite(struct inode* ip, char* buf, int n) {
  iunlock(ip);
  acquire(&cons.lock);
  for(int i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);
  return n;
}

void consoleinit(void) {
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  picenable(IRQ_KBD);
  ioapicenable(IRQ_KBD, 0);
}
