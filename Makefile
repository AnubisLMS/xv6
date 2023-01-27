K=kernel
U=user

OBJS = \
	$K/bio.o\
	$K/console.o\
	$K/exec.o\
	$K/file.o\
	$K/fs.o\
	$K/ide.o\
	$K/ioapic.o\
	$K/kalloc.o\
	$K/kbd.o\
	$K/lapic.o\
	$K/log.o\
	$K/main.o\
	$K/mp.o\
	$K/picirq.o\
	$K/pipe.o\
	$K/proc.o\
	$K/spinlock.o\
	$K/string.o\
	$K/swtch.o\
	$K/syscall.o\
	$K/sysfile.o\
	$K/sysproc.o\
	$K/timer.o\
	$K/trap.o\
	$K/uart.o\
	$K/vectors.o\
	$K/trapasm.o\
	$K/vm.o\
	$K/entry.o\

all: xv6.img

# If you would like to compile natively with docker, you can use these targets.
# To do this, run 'make docker'. This will compile the xv6.img in a docker container
# I would not recomend relying on docker to compile xv6 if you are not already proficent
# in using docker.
docker: os3224.image
	docker run -it --rm -v $$(pwd):/tmp/build -u $(shell id -u):$(shell id -g) os3224 make xv6.img

os3224.image:
	if ! docker image ls | grep os3224 &> /dev/null; then \
       docker build -t os3224 .; \
	fi

# Cross-compiling (e.g., on Mac OS X)
# TOOLPREFIX = i386-jos-elf

# Using native tools (e.g., on X86 Linux)
#TOOLPREFIX =

# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if i386-jos-elf-objdump -i 2>&1 | grep '^elf32-i386$$' >/dev/null 2>&1; \
	then echo 'i386-jos-elf-'; \
	elif objdump -i 2>&1 | grep 'elf32-i386' >/dev/null 2>&1; \
	then echo ''; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an i386-*-elf version of GCC/binutils." 1>&2; \
	echo "*** Is the directory with i386-jos-elf-gcc in your PATH?" 1>&2; \
	echo "*** If your i386-*-elf toolchain is installed with a command" 1>&2; \
	echo "*** prefix other than 'i386-jos-elf-', set your TOOLPREFIX" 1>&2; \
	echo "*** environment variable to that prefix and run 'make' again." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

QEMU = qemu-system-i386
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJFLAGS = -S -O binary -j .text
CFLAGS = -nostdinc -fno-pic -static -fno-builtin -fno-strict-aliasing -fvar-tracking -fvar-tracking-assignments -O0 -g -Wall -MD -gdwarf-2 -m32 -Werror -fno-omit-frame-pointer -ggdb
CFLAGS += -I. -mgeneral-regs-only
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide -I.
# FreeBSD ld wants ``elf_i386_fbsd''
LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

$K/bootblock:
	# bootmain must be optimized or it won't fit in the bootloader section
	$(CC) $(CFLAGS) -O -c $K/bootmain.c -o $K/bootmain.o
	$(CC) $(CFLAGS) -c $K/bootasm.S -o $K/bootasm.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $K/bootblock.o $K/bootasm.o $K/bootmain.o
	$(OBJCOPY) $(OBJFLAGS) $K/bootblock.o $K/bootblock
	perl $K/sign.pl $K/bootblock

$U/initcode:
	$(CC) $(CFLAGS) -c $U/initcode.S -o $U/initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $U/initcode.out $U/initcode.o
	$(OBJCOPY) $(OBJFLAGS) $U/initcode.out $U/initcode

$K/entryother:
	$(CC) $(CFLAGS) -c $K/entryother.S -o $K/entryother.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o $K/bootblockother.o $K/entryother.o
	$(OBJCOPY) $(OBJFLAGS) $K/bootblockother.o $K/entryother

$K/kernel: $(OBJS) $U/initcode $K/entryother
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/kernel $(OBJS) -b binary $U/initcode $K/entryother

ULIB = $U/ulib.o $U/usys.o $U/printf.o $U/umalloc.o

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -T $U/user.ld -o $@ $^

$U/_forktest: $U/forktest.o $(ULIB)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $U/_forktest $U/forktest.o $U/ulib.o $U/usys.o

$U/usys.o : $U/usys.S
	$(CC) $(CFLAGS) -c -o $U/usys.o $U/usys.S

mkfs/mkfs: mkfs/mkfs.c $K/fs.h
	gcc -Werror -Wall -I. -o mkfs/mkfs mkfs/mkfs.c

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: %.o

UPROGS=\
	$U/_cat\
	$U/_echo\
	$U/_forktest\
	$U/_grep\
	$U/_init\
	$U/_kill\
	$U/_ln\
	$U/_ls\
	$U/_mkdir\
	$U/_rm\
	$U/_sh\
	$U/_stressfs\
	$U/_usertests\
	$U/_wc\
	$U/_zombie\

fs.img: mkfs/mkfs README.md $(UPROGS)
	./mkfs/mkfs fs.img README.md $(UPROGS)

xv6.img: $K/bootblock $K/kernel fs.img
	dd if=$K/bootblock of=xv6.img
	dd if=$K/kernel of=xv6.img seek=1

-include $K/*.d $U/*.d

clean:
	rm -f */*.o */*.d */*.asm */*.sym $K/entryother \
	$K/bootblock $U/initcode $U/initcode.out $K/kernel xv6.img fs.img mkfs/mkfs \
	.gdbinit \
	$(UPROGS)

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

ifndef CPUS
CPUS := 1
endif

QEMUOPTS = \
	-drive file=xv6.img,media=disk,index=0,format=raw \
	-drive file=fs.img,index=1,media=disk,format=raw \
	-smp $(CPUS) -m 512 -display none -nographic

qemu: $K/kernel fs.img xv6.img
	$(QEMU) $(QEMUOPTS)

qemu-gdb: $K/kernel fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-vscode: $K/kernel fs.img xv6.img launch.json
	@echo "*** Now attach to qemu in the debug console ofb vscode." 1>&2
	@echo "file kernel/kernel" > .gdbinit
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

.PHONY: .gdbinit
.gdbinit: .gdbinit.tmpl
	cp .gdbinit.tmpl .gdbinit
	if [ -f ~/.gdbinit ]; then sed -i "s/# source/source/" ~/.gdbinit; fi
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

launch.json: launch.json.tmpl
	mkdir -p .vscode
	if [ -f .gdbinit ]; then rm .gdbinit; fi
	sed -i "s/^source/# source/" ~/.gdbinit
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > .vscode/$@
