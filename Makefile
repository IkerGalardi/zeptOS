RESULTS=build/opensbi-dynamic.bin \
        build/kernel.elf \
        build/fs.img

USER_PROGS = build/_cat \
             build/_echo \
             build/_forktest \
             build/_grep \
             build/_init \
             build/_kill \
             build/_ln \
             build/_ls \
             build/_mkdir \
             build/_rm \
             build/_sh \
             build/_stressfs \
             build/_usertests \
             build/_grind \
             build/_wc \
             build/_zombie \

KERNEL_OBJ = build/obj/kernel/entry.o \
             build/obj/kernel/start.o \
             build/obj/kernel/console.o \
             build/obj/kernel/printf.o \
             build/obj/kernel/uart.o \
             build/obj/kernel/kalloc.o \
             build/obj/kernel/spinlock.o \
             build/obj/kernel/string.o \
             build/obj/kernel/main.o \
             build/obj/kernel/vm.o \
             build/obj/kernel/proc.o \
             build/obj/kernel/swtch.o \
             build/obj/kernel/trampoline.o \
             build/obj/kernel/trap.o \
             build/obj/kernel/syscall.o \
             build/obj/kernel/sysproc.o \
             build/obj/kernel/bio.o \
             build/obj/kernel/fs.o \
             build/obj/kernel/log.o \
             build/obj/kernel/sleeplock.o \
             build/obj/kernel/file.o \
             build/obj/kernel/pipe.o \
             build/obj/kernel/exec.o \
             build/obj/kernel/sysfile.o \
             build/obj/kernel/kernelvec.o \
             build/obj/kernel/plic.o \
             build/obj/kernel/virtio_disk.o

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

.PRECIOUS: build/obj/user/%.o

all: $(RESULTS)

build/kernel.elf: $(KERNEL_OBJ) kernel/kernel.ld
	@echo "Linking   $@"
	@ ld.lld $(LDFLAGS) -T kernel/kernel.ld -o build/kernel.elf $(KERNEL_OBJ)

build/obj/kernel/%.o: kernel/%.S
	@echo "Compiling $<"
	@ clang --target=riscv64 -c -o $@ $< $(CFLAGS)

build/obj/kernel/%.o: kernel/%.c
	@echo "Compiling $<"
	@ clang --target=riscv64 -c -o $@ $< $(CFLAGS)

build/obj/user/%.o: user/%.S
	@echo "Compiling $<"
	@ clang --target=riscv64 -c -o $@ $< $(CFLAGS) -Ikernel/

build/obj/user/%.o: user/%.c
	@echo "Compiling $<"
	@ clang --target=riscv64 -c -o $@ $< $(CFLAGS) -Ikernel/

build/mkfs: tools/mkfs.c kernel/fs.h kernel/param.h
	@echo "Compiling $<"
	@ clang -o $@ $< -I.

ULIB = build/obj/user/ulib.o build/obj/user/usys.o build/obj/user/printf.o build/obj/user/umalloc.o

build/_%: build/obj/user/%.o $(ULIB)
	@echo "Linking $@"
	@ ld.lld $(LDFLAGS) -T user/user.ld -o $@ $< $(ULIB)

user/usys.S : user/usys.pl
	perl user/usys.pl > user/usys.S

build/opensbi-dynamic.bin:
	make -C firmware/ LLVM=1 PLATFORM=generic
	mv firmware/build/platform/generic/firmware/fw_dynamic.bin build/opensbi-dynamic.bin


build/fs.img: $(USER_PROGS) build/mkfs
	@echo "Building fs.img"
	@ build/mkfs build/fs.img $(USER_PROGS)

clean:
	rm -f $(RESULTS) build/obj/*/*.d build/obj/*/*.o user/usys.S $(USER_PROGS)

QEMUOPTS = -machine virt -bios none -kernel build/kernel.elf -m 128M -smp 3 -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

qemu-mmode:
	qemu-system-riscv64 $(QEMUOPTS)

qemu: $(RESULTS)
	qemu-system-riscv64 -bios build/opensbi-dynamic.bin -serial stdio