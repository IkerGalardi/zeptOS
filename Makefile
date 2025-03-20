CC=riscv64-linux-gnu-gcc
AS=riscv64-linux-gnu-as
LD=riscv64-linux-gnu-ld
OBJCOPY=riscv64-linux-gnu-objcopy
OBJDUMP=riscv64-linux-gnu-objdump

RESULTS=firmware/build/platform/generic/firmware/fw_dynamic.bin \
        kernel/kernel \
        fs.img

all: $(RESULTS)

firmware/build/platform/generic/firmware/fw_dynamic.bin:
	make -C firmware/ PLATFORM=generic CROSS_COMPILE=riscv64-linux-gnu-

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
CFLAGS += -I.

kernel/kernel: $(OBJS) kernel/kernel.ld
	@echo "LD kernel/kernel"
	@ $(LD) $(LDFLAGS) -T kernel/kernel.ld -o kernel/kernel $(OBJS)
	@ $(OBJDUMP) -S kernel/kernel > kernel/kernel.asm
	@ $(OBJDUMP) -t kernel/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel/kernel.sym

kernel/%.o: kernel/%.S
	@echo "CC $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^

kernel/%.o: kernel/%.c
	@echo "CC $^"
	@ $(CC) -c $(CFLAGS) -o $@ $^
