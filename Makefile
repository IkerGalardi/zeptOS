RESULTS=build/opensbi-dynamic.bin \
        build/kernel.elf

KERNEL_OBJ = build/obj/entry.o \
             build/obj/start.o \
             build/obj/console.o \
             build/obj/printf.o \
             build/obj/uart.o \
             build/obj/kalloc.o \
             build/obj/spinlock.o \
             build/obj/string.o \
             build/obj/main.o \
             build/obj/vm.o \
             build/obj/proc.o \
             build/obj/swtch.o \
             build/obj/trampoline.o \
             build/obj/trap.o \
             build/obj/syscall.o \
             build/obj/sysproc.o \
             build/obj/bio.o \
             build/obj/fs.o \
             build/obj/log.o \
             build/obj/sleeplock.o \
             build/obj/file.o \
             build/obj/pipe.o \
             build/obj/exec.o \
             build/obj/sysfile.o \
             build/obj/kernelvec.o \
             build/obj/plic.o \
             build/obj/virtio_disk.o

CFLAGS  =-Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -fno-common -nostdlib -mno-relax
CFLAGS += -fno-builtin-strncpy -fno-builtin-strncmp -fno-builtin-strlen -fno-builtin-memset
CFLAGS += -fno-builtin-memmove -fno-builtin-memcmp -fno-builtin-log -fno-builtin-bzero
CFLAGS += -fno-builtin-strchr -fno-builtin-exit -fno-builtin-malloc -fno-builtin-putc
CFLAGS += -fno-builtin-free
CFLAGS += -fno-builtin-memcpy -Wno-main -ffreestanding
CFLAGS += -fno-builtin-printf -fno-builtin-fprintf -fno-builtin-vprintf
CFLAGS += -I.
CFLAGS += -fno-stack-protector -fno-pie

LDFLAGS = -z max-page-size=4096

build/kernel.elf: $(KERNEL_OBJ) kernel/kernel.ld
	@echo "Linking   $@"
	@ ld.lld $(LDFLAGS) -T kernel/kernel.ld -o build/kernel $(KERNEL_OBJ)

build/obj/%.o: kernel/%.S
	@echo "Compiling $<"
	@ clang --target=riscv64 -c -o $@ $< $(CFLAGS)

build/obj/%.o: kernel/%.c
	@echo "Compiling $<"
	@clang --target=riscv64 -c -o $@ $< $(CFLAGS)

build/opensbi-dynamic.bin:
	make -C firmware/ LLVM=1 PLATFORM=generic
	mv firmware/build/platform/generic/firmware/fw_dynamic.bin build/opensbi-dynamic.bin

all: $(RESULTS)

clean:
	rm -f $(RESULTS) build/obj/*.d build/obj/*.o

qemu: $(RESULTS)
	qemu-system-riscv64 -bios build/opensbi-dynamic.bin -serial stdio