include config.mk
include check.mk

DEFINES=
MARCH_LP=
MARCH_SS=

ifeq ($(CONFIG_USER_LANDING_PAD), enabled)
	DEFINES+=-DCONFIG_USER_LANDING_PAD_ENABLED
	MARCH_LP=_zicfilp1p0
endif

ifeq ($(CONFIG_USER_SHADOW_STACK), software)
	DEFINES+=-DCONFIG_USER_SHADOW_STACK_SOFTWARE
endif

ifeq ($(CONFIG_USER_SHADOW_STACK), hardware)
	DEFINES+=-DCONFIG_USER_SHADOW_STACK_HARDWARE
	MARCH_SS=_zicfiss1p0
endif

MARCH=rv64gc$(MARCH_LP)$(MARCH_SS)

CC=clang --target=riscv64
AS=clang --target=riscv64
LD=ld.lld

RESULTS=firmware/build/platform/generic/firmware/fw_dynamic.bin \
        kernel/kernel \
        fs.img \
        tools/rvpte \
        tools/rvscause \
        tools/rvvaddr

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
CFLAGS += -I. -menable-experimental-extensions -march=$(MARCH)
CFLAGS += $(DEFINES)

CFLAGS_USER_EXTRA=
ifeq ($(CONFIG_USER_SHADOW_STACK), hardware)
	CFLAGS_USER_EXTRA=-fsanitize=shadow-call-stack -fcf-protection=return
endif
ifeq ($(CONFIG_USER_SHADOW_STACK), software)
	CFLAGS_USER_EXTRA=-fsanitize=shadow-call-stack
endif

kernel/kernel: $(OBJS) kernel/kernel.ld
	@echo "LD      kernel/kernel"
	@ $(LD) -nostdlib $(LDFLAGS) -T kernel/kernel.ld -o kernel/kernel $(OBJS)

kernel/%.o: kernel/%.S
	@echo "CC      $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^

kernel/%.o: kernel/%.c
	@echo "CC      $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^

tools/mkfs: tools/mkfs.c kernel/fs.h kernel/param.h
	@echo "CC      tools/mkfs"
	@ gcc -Werror -Wall -I. -o tools/mkfs tools/mkfs.c

tools/rvpte: tools/rvpte.c
	@echo "CC      tools/rvpte"
	@ gcc -Werror -Wall -I. -o tools/rvpte tools/rvpte.c

tools/rvscause: tools/rvscause.c
	@echo "CC      tools/rvscause"
	@ gcc -Werror -Wall -I. -o tools/rvscause tools/rvscause.c

tools/rvvaddr: tools/rvvaddr.c
	@echo "CC      tools/rvvaddr"
	@ gcc -Werror -Wall -I. -o tools/rvvaddr tools/rvvaddr.c

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
        user/_ciphertest


ifeq ($(CONFIG_USER_SHADOW_STACK), hardware)
	UPROGS+=user/_shadowtest
endif

ULIB = user/ulib.o user/usys.o user/printf.o user/umalloc.o

user/usys.S : user/usys.pl
	@echo "PERL    $^"
	@ perl user/usys.pl > user/usys.S

user/_ciphertest: user/ciphertest.o user/aes.o $(ULIB)
	@echo "LD      user/_ciphertest"
	@ $(LD) -nostdlib $(LDFLAGS) -T user/user.ld -o $@ $^

user/%.o: user/%.S
	@echo "CC      $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^

user/%.o: user/%.c
	@echo "CC      $^"
	@ $(CC) -c $(CFLAGS) $(CFLAGS_USER_EXTRA) -o $@ $^

user/_%: user/%.o $(ULIB)
	@echo "LD      $@"
	@ $(LD) -nostdlib $(LDFLAGS) -T user/user.ld -o $@ $^

fs.img: tools/mkfs README.md $(UPROGS)
	@echo "MKFS    fs.img"
	@ tools/mkfs fs.img README.md $(UPROGS)

QEMUOPTS  = -machine virt -bios firmware/build/platform/generic/firmware/fw_dynamic.bin -kernel kernel/kernel
QEMUOPTS += -m 128M -smp 4 -nographic -global virtio-mmio.force-legacy=false
QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
QEMUOPTS += -cpu rv64,zicfilp=true,zicfiss=true,zimop=true,zca=true,zcmop=true

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
	@ rm -f user/usys.S
	@ rm -f tools/mkfs
	@ rm -f tools/rvpte
	@ rm -f tools/rvscause
	@ rm -f tools/rvvaddr

env:
	docker run --name zeptosbuild --rm -v $(shell pwd):/code -w /code/ -it zeptosbuild 2> /dev/null || docker exec -it zeptosbuild sh

build-env:
	docker build . -t zeptosbuild
