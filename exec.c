#include "defs.h"
#include "elf.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "x86.h"

int exec(char* path, char** argv) {
  begin_op();
  struct inode* ip = namei(path);
  if(!ip) {
    end_op();
    return -1;
  }
  ilock(ip);

  // Check ELF header
  pde_t* pgdir = 0;
  struct elfhdr elf;
  if(readi(ip, (char*) &elf, 0, sizeof(elf)) < sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if(!(pgdir = setupkvm()))
    goto bad;

  // Load program into memory.
  unsigned int sz = 0;
  struct proghdr ph;
  for(int i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
    if(readi(ip, (char*) &ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(loaduvm(pgdir, (char*) ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2 * PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*) (sz - 2 * PGSIZE));

  // Push argument strings, prepare rest of stack in ustack.
  unsigned int argc, sp, ustack[3 + MAXARG + 1];
  for(argc = 0, sp = sz; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3 + argc] = sp;
  }
  ustack[3 + argc] = 0;

  ustack[0] = 0xffffffff; // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc + 1) * 4; // argv pointer

  sp -= (3 + argc + 1) * 4;
  if(copyout(pgdir, sp, ustack, (3 + argc + 1) * 4) < 0)
    goto bad;

  // Save program name for debugging.
  char* last = path;
  for(; *path; path++)
    if(*path == '/')
      last = path + 1;
  safestrcpy(proc->name, last, sizeof(proc->name));

  // Commit to the user image.
  pde_t* oldpgdir = proc->pgdir;
  proc->pgdir = pgdir;
  proc->sz = sz;
  proc->tf->eip = elf.entry; // main
  proc->tf->esp = sp;
  switchuvm(proc);
  freevm(oldpgdir);
  return 0;

bad:
  if(pgdir)
    freevm(pgdir);
  if(ip) {
    iunlockput(ip);
    end_op();
  }
  return -1;
}
