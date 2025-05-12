CC=clang --target=riscv64
AS=clang --target=riscv64
LD=ld.lld
OBJCOPY=llvm-objcopy
OBJDUMP=llvm-objdump

RESULTS=firmware/build/platform/generic/firmware/fw_dynamic.bin \
        kernel/kernel \
        fs.img

all: $(RESULTS)

firmware/build/platform/generic/firmware/fw_dynamic.bin:
	make -C firmware/ PLATFORM=generic LLVM=1

# Kernel building

OBJS = kernel/entry.o \
       kernel/start.o \
       kernel/console.o \
       kernel/printf.o \
       kernel/uart.o \
       kernel/kalloc.o \
       kernel/spinlock.o \
       kernel/string.o \
       kernel/main.o \
       kernel/vm.o \
       kernel/proc.o \
       kernel/swtch.o \
       kernel/trampoline.o \
       kernel/trap.o \
       kernel/syscall.o \
       kernel/sysproc.o \
       kernel/bio.o \
       kernel/fs.o \
       kernel/log.o \
       kernel/sleeplock.o \
       kernel/file.o \
       kernel/pipe.o \
       kernel/exec.o \
       kernel/sysfile.o \
       kernel/kernelvec.o \
       kernel/plic.o \
       kernel/virtio_disk.o \
       kernel/sbi.o \
       kernel/dtb.o \
       kernel/globals.o

CFLAGS  = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD -mcmodel=medany -fno-common -nostdlib -mno-relax
CFLAGS += -fno-builtin-strncpy -fno-builtin-strncmp -fno-builtin-strlen -fno-builtin-memset
CFLAGS += -fno-builtin-memmove -fno-builtin-memcmp -fno-builtin-log -fno-builtin-bzero
CFLAGS += -fno-builtin-strchr -fno-builtin-exit -fno-builtin-malloc -fno-builtin-putc
CFLAGS += -fno-builtin-free -fno-builtin-memcpy -Wno-main -fno-stack-protector
CFLAGS += -fno-builtin-printf -fno-builtin-fprintf -fno-builtin-vprintf
CFLAGS += -I. -menable-experimental-extensions -march=rv64gc_zicfilp1p0

kernel/kernel: $(OBJS) kernel/kernel.ld
	@echo "LD      kernel/kernel"
	@ $(LD) -nostdlib $(LDFLAGS) -T kernel/kernel.ld -o kernel/kernel $(OBJS)
	@echo "OBJDUMP kernel/kernel.asm"
	@ $(OBJDUMP) -S kernel/kernel > kernel/kernel.asm
	@echo "OBJDUMP kernel/kernel.sym"
	@ $(OBJDUMP) -t kernel/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel/kernel.sym

kernel/%.o: kernel/%.S
	@echo "CC      $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^

kernel/%.o: kernel/%.c
	@echo "CC      $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^

mkfs/mkfs: mkfs/mkfs.c kernel/fs.h kernel/param.h
	@echo "CC      mkfs/mkfs"
	@ gcc -Werror -Wall -I. -o mkfs/mkfs mkfs/mkfs.c

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: user/%.o

UPROGS= user/_cat \
        user/_echo \
        user/_forktest \
        user/_grep \
        user/_init \
        user/_kill \
        user/_ln \
        user/_ls \
        user/_mkdir \
        user/_rm \
        user/_sh \
        user/_stressfs \
        user/_usertests \
        user/_grind \
        user/_wc \
        user/_zombie \

ULIB = user/ulib.o user/usys.o user/printf.o user/umalloc.o

user/usys.S : user/usys.pl
	@echo "PERL    $^"
	@ perl user/usys.pl > user/usys.S

user/%.o: user/%.S
	@echo "CC      $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^

user/%.o: user/%.c
	@echo "CC      $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^

user/_%: user/%.o $(ULIB)
	@ $(LD) -nostdlib $(LDFLAGS) -T user/user.ld -o $@ $^
	@ $(OBJDUMP) -S $@ > user/$*.asm
	@ $(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > user/$*.sym

fs.img: mkfs/mkfs README.md $(UPROGS)
	@echo "MKFS    fs.img"
	@ mkfs/mkfs fs.img README.md $(UPROGS)

QEMUOPTS  = -machine virt -bios firmware/build/platform/generic/firmware/fw_dynamic.bin -kernel kernel/kernel
QEMUOPTS += -m 128M -smp 4 -nographic -global virtio-mmio.force-legacy=false
QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
QEMUOPTS += -cpu rv64,zicfilp=true

qemu: $(RESULTS)
	qemu-system-riscv64 $(QEMUOPTS)

qemu-gdb: kernel/kernel fs.img
	qemu-system-riscv64 $(QEMUOPTS) -S -s

clean:
	@ rm -f kernel/*.o
	@ rm -f kernel/*.d
	@ rm -f user/*.o
	@ rm -f user/_*
	@ rm -f user/*.d
	@ rm -f *.sym
	@ rm -f *.asm
	@ rm -f fs.img

env:
	docker run -v $(shell pwd):/code -w /code/ -it zeptosbuild

build-env:
	docker build . -t zeptosbuild
