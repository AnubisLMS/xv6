#ifndef XV6_X86_H
#define XV6_X86_H

// Routines to let C code use special x86 instructions.

static inline unsigned char inb(unsigned short port) {
  unsigned char data;

  asm volatile("in %1,%0" : "=a"(data) : "d"(port));
  return data;
}

static inline void insl(int port, void* addr, int cnt) {
  asm volatile("cld; rep insl"
               : "=D"(addr), "=c"(cnt)
               : "d"(port), "0"(addr), "1"(cnt)
               : "memory", "cc");
}

static inline void outb(unsigned short port, unsigned char data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outw(unsigned short port, unsigned short data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outsl(int port, const void* addr, int cnt) {
  asm volatile("cld; rep outsl"
               : "=S"(addr), "=c"(cnt)
               : "d"(port), "0"(addr), "1"(cnt)
               : "cc");
}

static inline void stosb(void* addr, int data, int cnt) {
  asm volatile("cld; rep stosb"
               : "=D"(addr), "=c"(cnt)
               : "0"(addr), "1"(cnt), "a"(data)
               : "memory", "cc");
}

static inline void stosl(void* addr, int data, int cnt) {
  asm volatile("cld; rep stosl"
               : "=D"(addr), "=c"(cnt)
               : "0"(addr), "1"(cnt), "a"(data)
               : "memory", "cc");
}

struct segdesc;

static inline void lgdt(struct segdesc* p, int size) {
  volatile unsigned short pd[3];

  pd[0] = size - 1;
  pd[1] = (unsigned int) p;
  pd[2] = (unsigned int) p >> 16;

  asm volatile("lgdt (%0)" : : "r"(pd));
}

struct gatedesc;

static inline void lidt(struct gatedesc* p, int size) {
  volatile unsigned short pd[3];

  pd[0] = size - 1;
  pd[1] = (unsigned int) p;
  pd[2] = (unsigned int) p >> 16;

  asm volatile("lidt (%0)" : : "r"(pd));
}

static inline void ltr(unsigned short sel) {
  asm volatile("ltr %0" : : "r"(sel));
}

static inline unsigned int readeflags(void) {
  unsigned int eflags;
  asm volatile("pushfl; popl %0" : "=r"(eflags));
  return eflags;
}

static inline void loadgs(unsigned short v) {
  asm volatile("movw %0, %%gs" : : "r"(v));
}

static inline void cli(void) {
  asm volatile("cli");
}

static inline void sti(void) {
  asm volatile("sti");
}

static inline void hlt(void) {
  asm volatile("hlt");
}

static inline unsigned int xchg(volatile unsigned int* addr,
    unsigned int newval) {
  unsigned int result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1"
               : "+m"(*addr), "=a"(result)
               : "1"(newval)
               : "cc");
  return result;
}

static inline unsigned int rcr2(void) {
  unsigned int val;
  asm volatile("movl %%cr2,%0" : "=r"(val));
  return val;
}

static inline void lcr3(unsigned int val) {
  asm volatile("movl %0,%%cr3" : : "r"(val));
}

// PAGEBREAK: 36
// Layout of the trap frame built on the stack by the
// hardware and by trapasm.S, and passed to trap().
struct trapframe {
  // registers as pushed by pusha
  unsigned int edi;
  unsigned int esi;
  unsigned int ebp;
  unsigned int oesp; // useless & ignored
  unsigned int ebx;
  unsigned int edx;
  unsigned int ecx;
  unsigned int eax;

  // rest of trap frame
  unsigned short gs;
  unsigned short padding1;
  unsigned short fs;
  unsigned short padding2;
  unsigned short es;
  unsigned short padding3;
  unsigned short ds;
  unsigned short padding4;
  unsigned int trapno;

  // below here defined by x86 hardware
  unsigned int err;
  unsigned int eip;
  unsigned short cs;
  unsigned short padding5;
  unsigned int eflags;

  // below here only when crossing rings, such as from user to kernel
  unsigned int esp;
  unsigned short ss;
  unsigned short padding6;
};

#endif // XV6_X86_H
