# VSCode Integration

## Compiling
If you are using the class vm, to compile, just run `make xv6.img` in the project directory. That will build xv6.img. Assuimging that was successful, you can then run xv6 by running `make qemu`. If you make changes to any xv6, you will likely need to first clean out the "stale" binaries before rebuilding xv6.img. You can clean your build environment with `make clean`.

## Debugging 
You will then want to navigate to
run `make clean xv6.img qemu-vscode` once the gdb server has started, it will wait for connections. You can then 
navigate to your debug console in VSCode and select gdb from the gear icon. You should see an "Attach to QEMU" profile 
available. When you run this, you should connect to the gdb server. Go ahead and try to set up breakpoints and whatnot. 

## Compiling with Docker (if you want to)
If you are already a linux power user, you can compile natively with docker. You can compile xv6 with docker my running `make docker-build`. That will run an ubunut 16.04 container that will compile xv6.img for you. I would not recommend relying on this unless you are already profcient with docker.

## Dependencies (if you are not on the VM)
If you are compiling natively, will need to add a plugin to your vscode. To install the necessary dependencies for debugging in vscode, press `ctrl p` then enter `ext install webfreak.debug`. 
That will install [this](https://github.com/WebFreak001/code-debug) vscode module. You will also need to install
gdb and lldb if you haven't already. On debian that is `sudo apt update && sudo apt install -y gdb lldb`, on arch 
(btw) `sudo pacman -S gdb lldb`, and alpine `sudo apk add --update gdb lldb`.


# Old stuff

xv6 is a re-implementation of Dennis Ritchie's and Ken Thompson's Unix
Version 6 (v6).  xv6 loosely follows the structure and style of v6,
but is implemented for a modern x86-based multiprocessor using ANSI C.

ACKNOWLEDGMENTS

xv6 is inspired by John Lions's Commentary on UNIX 6th Edition (Peer
to Peer Communications; ISBN: 1-57398-013-7; 1st edition (June 14,
2000)). See also http://pdos.csail.mit.edu/6.828/2014/xv6.html, which
provides pointers to on-line resources for v6.

xv6 borrows code from the following sources:
    JOS (asm.h, elf.h, mmu.h, bootasm.S, ide.c, console.c, and others)
    Plan 9 (entryother.S, mp.h, mp.c, lapic.c)
    FreeBSD (ioapic.c)
    NetBSD (console.c)

The following people have made contributions:
    Russ Cox (context switching, locking)
    Cliff Frey (MP)
    Xiao Yu (MP)
    Nickolai Zeldovich
    Austin Clements

In addition, we are grateful for the bug reports and patches contributed by
Silas Boyd-Wickizer, Peter Froehlich, Shivam Handa, Anders Kaseorg, Eddie
Kohler, Yandong Mao, Hitoshi Mitake, Carmi Merimovich, Joel Nider, Greg Price,
Eldar Sehayek, Yongming Shen, Stephen Tu, and Zouchangwei.

The code in the files that constitute xv6 is
Copyright 2006-2014 Frans Kaashoek, Robert Morris, and Russ Cox.

ERROR REPORTS

If you spot errors or have suggestions for improvement, please send
email to Frans Kaashoek and Robert Morris (kaashoek,rtm@csail.mit.edu). 

BUILDING AND RUNNING XV6

To build xv6 on an x86 ELF machine (like Linux or FreeBSD), run "make".
On non-x86 or non-ELF machines (like OS X, even on x86), you will
need to install a cross-compiler gcc suite capable of producing x86 ELF
binaries.  See http://pdos.csail.mit.edu/6.828/2014/tools.html.
Then run "make TOOLPREFIX=i386-jos-elf-".

To run xv6, install the QEMU PC simulators.  To run in QEMU, run "make qemu".

To create a typeset version of the code, run "make xv6.pdf".  This
requires the "mpage" utility.  See http://www.mesa.nl/pub/mpage/.
